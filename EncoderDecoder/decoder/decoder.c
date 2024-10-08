
#include "decoder.h"


//threads
typedef struct {
	ProtectionData* protection;
	BlockData* decoded_data;
	size_t start;
	size_t end;
} ThreadDecodeParams;


DWORD WINAPI thread_decode_block(LPVOID param) {
	ThreadDecodeParams* params = (ThreadDecodeParams*)param;

	for (size_t i = params->start; i < params->end; i++) {
		char* block = block_decoder(params->protection[i]);
		int num_bytes = BLOCK_SIZE / 8;
		memcpy(params->decoded_data[i].data, block, num_bytes);
	}

	return 0;
}

//files

int write_memory_to_file(const char* filename, const char* data, unsigned long long int length) {
	FILE* file = NULL;
	errno_t err = fopen_s(&file, filename, "wb"); // פתיחת הקובץ במצב כתיבה בינארית
	if (err != 0 || file == NULL) {
		perror("Error opening file");
		return -1;
	}

	size_t written_size = fwrite(data, 1, length, file);
	if (written_size != length) {
		perror("Error writing to file");
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0; // הצלחה
}


ProtectionData* read_protection_data_from_file(const char* file_name, long* number_of_blocks)
{
	FILE* file = NULL;
	errno_t err;

	// פתיחת קובץ בינארי במצב כתיבה
	err = fopen_s(&file, file_name, "rb");

	// בדיקת אם הקובץ נפתח בהצלחה
	if (err != 0) {
		perror("Error opening file");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file); // גודל הקובץ בבתים
	rewind(file); // חזרה לתחילת הקובץ
	*number_of_blocks = file_size / sizeof(ProtectionData);

	ProtectionData* protection = (ProtectionData*)malloc(sizeof(ProtectionData) * (*number_of_blocks));
	for (size_t i = 0; i < *number_of_blocks; i++)
	{
		fread(&protection[i], sizeof(ProtectionData), 1, file);

	}
	fclose(file);
	return protection;
}

char* remove_bin_extension(const char* file_path) {
	const char* extension = ".bin";
	size_t len = strlen(file_path);
	size_t ext_len = strlen(extension);

	if (len > ext_len && strcmp(file_path + len - ext_len, extension) == 0) {
		char* new_file_path = (char*)malloc(len - ext_len + 1);
		if (new_file_path == NULL) {
			perror("Error allocating memory");
			return NULL;
		}
		strncpy_s(new_file_path, len - ext_len + 1, file_path, len - ext_len);
		new_file_path[len - ext_len] = '\0';
		return new_file_path;
	}
	return _strdup(file_path);
}

int has_bin_extension(const char* filename) {
	const char* extension = ".bin";
	size_t len_filename = strlen(filename);
	size_t len_extension = strlen(extension);

	// בדוק אם אורך המחרוזת קצר מאורך הסיומת
	if (len_filename < len_extension) {
		return 0;
	}

	// השווה את סיומת הקובץ לסיומת .bin
	if (strcmp(filename + len_filename - len_extension, extension) == 0) {
		return 1;
	}
	else {
		return 0;
	}
}


//logic of decoder

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


int hamming1_result(char* encoded_data)
{
	int result = 0;
	int num_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	for (size_t i = 1; i <= num_bits; i++)
	{
		int this_parity_bit = 0;
		for (size_t j = (size_t)pow(2, i - 1) + 1; j <= BLOCK_SIZE + num_bits; j++) {
			if (j & (size_t)pow(2, i - 1))
			{
				this_parity_bit ^= get_bit_value(encoded_data, j);
			}
		}
		if (this_parity_bit != get_bit_value(encoded_data, (size_t)pow(2, i - 1)))
		{
			result |= 1 << (i - 1);
		}
	}
	return result;
}

int hamming2_result(char* data, MyType hamming)
{
	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block((BLOCK_SIZE + number_of_parity_bits_for_block) / 2);
	int result = 0;
	for (size_t i = 1; i <= number_of_parity_bits; i++)
	{
		int this_parity_bit = 0;
		for (int j = (int)pow(2, i - 1) + 1; j <= (BLOCK_SIZE + number_of_parity_bits_for_block) / 2 + number_of_parity_bits; j++) {
			int index = get_adjusted_index(j, 2);

			if (index != -1 && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}
		if (this_parity_bit != get_bit_value(&hamming, i))
		{
			result |= 1 << (i - 1);

		}

	}
	return get_adjusted_index(result, 2);
}



char* original_data(char* encoded_data)
{

	char* data = (char*)calloc(NUM_BYTES(HAMMING1_SIZE), sizeof(char));
	if (data == NULL) {
		// Handle memory allocation failure
		return NULL;
	}
	int index = 1;
	for (size_t i = 1; i <= HAMMING1_SIZE; i++)
	{

		if (!IS_POWER_OF_TWO(i))
		{
			if (get_bit_value(encoded_data, i))
			{

				data[(index - 1) / 8] |= 1 << (8 - (index) % 8);
				if (index % 8 == 0)
					data[(index - 1) / 8] |= 1;

			}
			index++;
		}
	}
	return data;
}


void change_bit_in_data(char* data, int place)
{
	if (place >= 0 && place <= HAMMING1_SIZE)
		data[(place - 1) / 8] ^= (1 << (8 - place % 8));
	if (place % 8 == 0)
	{
		data[(place - 1) / 8] ^= 1;
	}
}


int try_change_bit(char* data, int place)
{

	change_bit_in_data(data, place);
	return  hamming1_result(data);

}


void two_bits_swapped(char* encoded_data, MyType hamming2)
{
	int error = hamming2_result(encoded_data, hamming2);
	try_change_bit(encoded_data, error);
	if (try_change_bit(encoded_data, error - 1) == 0)
		return;
	try_change_bit(encoded_data, error - 1);
	try_change_bit(encoded_data, error + 1);
}

char* block_decoder(ProtectionData pd)
{
	unsigned int error_place = hamming1_result(pd.hamming1);
	if (error_place == 0)
	{
		return original_data(pd.hamming1);
	}
	else
	{
		
		if (((pd.parityBit & 1) ^ (pd.parityBit >> 1) & 1))
		{
			printf("error in parity bit\n");
		}

		if (pd.parityBit != parity_bit_of_data(pd.hamming1, HAMMING1_SIZE))
		{

			printf("bit flip in %d\n", error_place);
			change_bit_in_data(pd.hamming1, error_place);
		}
		else
		{
			printf("two bits swapped\n");
			two_bits_swapped(pd.hamming1, pd.hamming2);
		}

	}

	return original_data(pd.hamming1);
}

char* decode(ProtectionData* protection, int number_of_blocks) {

	clock_t start, end;
	start = clock();
	BlockData* decoded_data = (BlockData*)malloc(number_of_blocks * sizeof(BlockData));

	// התאמת מספר הטרדים לגודל הדאטה
	size_t optimal_thread_count = (number_of_blocks < 8) ? number_of_blocks : 8;

	HANDLE* threads = (HANDLE*)malloc(optimal_thread_count * sizeof(HANDLE));
	ThreadDecodeParams* thread_params = (ThreadDecodeParams*)malloc(optimal_thread_count * sizeof(ThreadDecodeParams));

	size_t blocks_per_thread = number_of_blocks / optimal_thread_count;
	size_t remaining_blocks = number_of_blocks % optimal_thread_count;

	// יצירת הטרדים בהתאם לגודל הדאטה
	for (size_t i = 0; i < optimal_thread_count; i++) {
		thread_params[i].protection = protection;
		thread_params[i].decoded_data = decoded_data;
		thread_params[i].start = i * blocks_per_thread;
		thread_params[i].end = (i + 1) * blocks_per_thread;

		if (i == optimal_thread_count - 1) {
			thread_params[i].end += remaining_blocks;  // בלוקים שנותרו ייכנסו לטרד האחרון
		}

		threads[i] = CreateThread(NULL, 0, thread_decode_block, &thread_params[i], 0, NULL);
		if (threads[i] == NULL) {
			perror("Error creating thread");
			free(decoded_data);
			free(protection);
			free(threads);
			free(thread_params);
			return NULL;
		}
	}
	WaitForMultipleObjects(optimal_thread_count, threads, TRUE, INFINITE);

	// סגירת הטרדים
	for (size_t i = 0; i < optimal_thread_count; i++) {
		CloseHandle(threads[i]);
	}

	// שחרור הזיכרון
	free(threads);
	free(thread_params);
	end = clock();
	double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
	FILE* file = fopen("C:\\encode_decode\\decode\\time.txt", "a");
	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	// כתיבת זמן הריצה לקובץ
	fprintf(file, "Time taken by decoder when block size is %d: %f seconds\n", BLOCK_SIZE, time_spent);

	// סגירת הקובץ
	fclose(file);
	return (char*)decoded_data;
}

void decode_files(const char* file_name)
{
	long number_of_blocks = 0;
	ProtectionData* protection = read_protection_data_from_file(file_name, &number_of_blocks);
	if (protection == NULL)
		return NULL;
	char* data = decode(protection, number_of_blocks);
	write_memory_to_file(remove_bin_extension(file_name), data, number_of_blocks * BLOCK_SIZE / 8);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: encode <input_file>\n");
		return 1;
	}
	if (!has_bin_extension(argv[1]))
	{
		printf("not valid file. file must have bin extension\n");
		return 1;
	}
	const char* file_name = argv[1];
	decode_files(file_name);
}