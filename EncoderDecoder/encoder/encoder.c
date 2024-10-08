
#include "encoder.h"

//threads

#define NUM_THREADS 4

typedef struct {
	char* data;
	size_t start;
	size_t end;
} ThreadData;

typedef struct {
	char* data;
	size_t start_block;
	size_t end_block;
} ThreadParams;

DWORD WINAPI thread_encode_block(LPVOID lpParam) {
	ThreadParams* params = (ThreadParams*)lpParam;
	char* data = params->data;
	for (size_t i = params->start_block; i < params->end_block; i++) {
		block_encode(data, i);
		data += NUM_BYTES(BLOCK_SIZE);  // מעבר לבלוק הבא
	}
	free(params);
	return 0;
}

//files
char* read_file_to_memory(const char* filename, unsigned long long int* out_length) {
	FILE* file = NULL;
	errno_t err = fopen_s(&file, filename, "rb"); // פתיחת הקובץ במצב קריאה בינארית
	if (err != 0 || file == NULL) {
		perror("Error opening file");
		return NULL;
	}

	// קביעת מיקום הסיום של הקובץ
	fseek(file, 0, SEEK_END);
	long length = ftell(file); // קבלת גודל הקובץ
	fseek(file, 0, SEEK_SET);  // החזרת מיקום הקריאה להתחלה

	if (length < 0) {
		perror("Error determining file size");
		fclose(file);
		return NULL;
	}

	// הקצאת זיכרון עבור התוכן של הקובץ
	char* buffer = (char*)malloc(length + 1); // +1 עבור תו הסיום '\0'
	if (buffer == NULL) {
		perror("Error allocating memory");
		fclose(file);
		return NULL;
	}

	// קריאת התוכן לקובץ לתוך הזיכרון
	size_t read_size = fread(buffer, 1, length, file);
	if (read_size != length) {
		perror("Error reading file");
		free(buffer);
		fclose(file);
		return NULL;
	}

	buffer[length] = '\0'; // הוספת תו סיום מחרוזת

	fclose(file);
	if (out_length) {
		*out_length = length;
	}
	return buffer;
}


char* add_bin_extension(const char* file_path) {
	const char* extension = ".bin";
	size_t len = strlen(file_path) + strlen(extension) + 1;
	char* new_file_path = (char*)malloc(len);

	if (new_file_path == NULL) {
		perror("Error allocating memory");
		return NULL;
	}

	strcpy_s(new_file_path, len, file_path);
	strcat_s(new_file_path, len, extension);

	return new_file_path;
}


void write_protection_data_to_file(const char* file_name, ProtectionData* protection, int final_blocks_num)
{
	FILE* file = NULL;
	errno_t err;

	// פתיחת קובץ בינארי במצב כתיבה
	err = fopen_s(&file, file_name, "wb");

	// בדיקת אם הקובץ נפתח בהצלחה
	if (err != 0) {
		perror("Error opening file");
		return 1;
	}

	for (size_t i = 0; i < final_blocks_num; i++)
	{
		size_t result = fwrite(&protection[i], sizeof(ProtectionData), 1, file);
		if (result != 1) {
			perror("Error writing to file");
			fclose(file);
			return 1;
		}
	}

	fclose(file);
}

//logic of encoder

int calculate_number_of_parity_bits_for_block(int size)
{
	int count = 0;
	while ((1 << count) < (size + count + 1)) {
		count++;
	}

	return count;
}


int get_adjusted_index(int index, int skipped) {

	int adjusted_index = 1;
	for (size_t i = 1; i < index; ++i) {
		if (!IS_POWER_OF_TWO(i)) {
			++adjusted_index;
		}
	}
	if (skipped == 1)
		return adjusted_index;
	return adjusted_index * 2 - 1;
}

int get_bit_value(char* data, int bitPosition) {

	bitPosition--;

	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8);
	unsigned char byte = data[byteIndex];
	unsigned char mask = 1 << (bitIndex);
	int bitValue = (byte & mask) >> bitIndex;
	return bitValue;
}



//calculate the parity bit of the block (Multy_Xor)
MyType parity_bit_of_data(char* data, size_t length) {
	int parity = 0;

	for (size_t i = 0; i < NUM_BYTES(length); ++i) {
		unsigned char byte = data[i];
		size_t bits_in_current_byte = (i == NUM_BYTES(length) - 1) ? (length % 8) : 8;
		for (size_t bit = 0; bit < 8; ++bit) {
			uint8_t bit_value = (byte >> bit) & 1;
			parity ^= bit_value;
		}
	}

	if (parity == 1)
		return 3;
	return 0;
}




void hamming1_encode(char* data, int index) {
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;

	//fill the array of hamming1 with zeros

	memset(pd[index].hamming1, 0, sizeof(pd[index].hamming1));


	//go through all the bits in the array
	for (size_t i = 1; i <= num_bits; i++) {
		//if its a parity bit
		if (IS_POWER_OF_TWO(i)) {
			int this_parity_bit = 0;
			int index_2 = get_adjusted_index(i+1, 1);
			//go through all bits from the current bit onwards
			for (size_t j = i + 1; j <= BLOCK_SIZE + number_of_parity_bits; j++) {
				
				//check if the current bit should be calculated as a part of this parity bit
				if (EQUALS_TO_PARITY_BIT(i, j)) {
					this_parity_bit ^= get_bit_value(data, index_2);
				}
				if (!IS_POWER_OF_TWO(j))
					index_2++;
			}
			//save the parity bit value in the appropriate place
			if (this_parity_bit) {
				(pd[index].hamming1)[(i - 1) / 8] |= 1 << ((8 - i) % 8);

			}
		}

		//if its not a parity bit
		else
		{
			//save the bit value in the appropriate place
			if (get_bit_value(data, get_adjusted_index(i, 1))) {
				pd[index].hamming1[(i - 1) / 8] |= 1 << ((8 - i) % 8);
			}
		}

	}

}


MyType hamming2_encode(const void* data) {

	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	int number_of_parity_bits_for_hamming2 = calculate_number_of_parity_bits_for_block((BLOCK_SIZE + number_of_parity_bits_for_block) / 2);

	// Initialize the result with zero
	MyType encoded_data = 0;


	//go through all the parity bits
	for (int i = 1; i <= number_of_parity_bits_for_hamming2; i++) {
		int this_parity_bit = 0;

		//go through all the bits from the current parity bits and on
		for (int j = (int)pow(2, i - 1); j <= (BLOCK_SIZE + number_of_parity_bits_for_block) / 2 + number_of_parity_bits_for_hamming2; j++) {
			int index = get_adjusted_index(j, 2);

			//check if the current bit should be calculated as a part of this parity bit
			if (!IS_POWER_OF_TWO(j) && EQUALS_TO_PARITY_BIT(j, (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}

		if (this_parity_bit) {
			encoded_data |= (MyType)(1 << ((8 * sizeof(MyType) - i) % (8 * sizeof(MyType))));
		}
	}

	return encoded_data;
}



void block_encode(void* block, int index)
{
	hamming1_encode(block, index);
	pd[index].parityBit = parity_bit_of_data((pd[index].hamming1), HAMMING1_SIZE);
	pd[index].hamming2 = hamming2_encode(pd[index].hamming1);
}




void encode(char* data, long length, char* bin_file_name) {
	clock_t start, end;
	start = clock();
	if (data == NULL) {
		pd = NULL;
		return;
	}

	size_t full_blocks = length / BLOCK_SIZE;
	size_t last_block_size = length % BLOCK_SIZE;
	size_t final_blocks_num = full_blocks;
	if (last_block_size)
		final_blocks_num++;

	pd = (ProtectionData*)malloc(final_blocks_num * sizeof(ProtectionData));
	if (pd == NULL) {
		perror("Error allocating memory");
		return;
	}

	size_t num_threads = (full_blocks < 8) ? full_blocks : 8;
	HANDLE* threads = (HANDLE*)malloc(num_threads * sizeof(HANDLE));  // מערך הטרדים
	if (threads == NULL) {
		perror("Error allocating memory for threads");
		free(pd);
		return;
	}

	size_t blocks_per_thread = full_blocks / num_threads;
	size_t remaining_blocks = full_blocks % num_threads;

	// יצירת טרדים לכל חלק מהבלוקים
	for (int i = 0; i < num_threads; i++) {
		size_t start_block = i * blocks_per_thread;
		size_t end_block = start_block + blocks_per_thread;
		if (i == num_threads - 1) {
			end_block += remaining_blocks;  // לטרד האחרון נותרו בלוקים נוספים
		}

		ThreadParams* params = (ThreadParams*)malloc(sizeof(ThreadParams));
		if (params == NULL) {
			perror("Error allocating memory for thread parameters");
			free(pd);
			free(threads);
			return;
		}

		params->data = data + start_block * (BLOCK_SIZE / 8);
		params->start_block = start_block;
		params->end_block = end_block;

		// יצירת הטרד
		threads[i] = CreateThread(NULL, 0, thread_encode_block, params, 0, NULL);
		if (threads[i] == NULL) {
			perror("Error creating thread");
			free(pd);
			free(threads);
			free(params);
			return;
		}
	}

	WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);  // המתנה לכל הטרדים

	for (int i = 0; i < num_threads; i++) {
		CloseHandle(threads[i]);  // סגירת כל טרד
	}

	// טיפול בבלוק האחרון אם יש בלוק חלקי
	if (last_block_size > 0) {
		char* last_block = (char*)malloc(NUM_BYTES(BLOCK_SIZE));
		if (last_block == NULL) {
			perror("Error allocating memory for last block");
			free(pd);
			free(threads);
			return;
		}

		memset(last_block, 0, NUM_BYTES(BLOCK_SIZE));
		memcpy(last_block, data + full_blocks * (BLOCK_SIZE / 8), NUM_BYTES(last_block_size));
		block_encode(last_block, full_blocks);
		free(last_block);
	}

	// כתיבה לקובץ
	write_protection_data_to_file(bin_file_name, pd, final_blocks_num);

	// שחרור זיכרון
	free(pd);
	free(threads);

	end = clock();
	double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
	FILE* file = fopen("C:\\encode_decode\\encode\\time.txt", "a");
	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	// כתיבת זמן הריצה לקובץ
	fprintf(file, "Time taken by encoder when block size is %d: %f seconds\n", BLOCK_SIZE, time_spent);

	// סגירת הקובץ
	fclose(file);
}


long encode_files(const char* file_name)
{
	unsigned long long int data_length = 0;
	char* data = read_file_to_memory(file_name, &data_length);
	encode(data, data_length * 8, add_bin_extension(file_name));
	remove(file_name);
	return data_length;
}


int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: encode <input_file>\n");
		return 1;
	}

	const char* file_name = argv[1];
	encode_files(file_name);
}



void binary_represent(void* data, int bits_number) {
	unsigned char* byte_ptr = (unsigned char*)data;

	int total_bytes = (bits_number + 7) / 8;

	for (int i = 0; i < total_bytes; ++i) {
		unsigned char byte = byte_ptr[i];

		int bits_to_print = (i == total_bytes - 1) ? (bits_number % 8) : 8;
		if (bits_to_print == 0) bits_to_print = 8; // אם זה byte הראשון

		for (int bit = bits_to_print - 1; bit >= 0; --bit) {
			printf("%d ", (byte >> bit) & 1);
		}
	}

	printf("\n");
}




