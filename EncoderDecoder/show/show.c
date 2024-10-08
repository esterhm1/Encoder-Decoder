#include "show.h"
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
	/*if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1;
	}*/

	unsigned char byte = data[byteIndex];
	unsigned char mask = 1 << (bitIndex);
	int bitValue = (byte & mask) >> bitIndex;
	return bitValue;
}

char* original_data(char* encoded_data)
{
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;
	size_t num_bytes = (num_bits + 7) / 8; // Round up to nearest byte

	char* data = (char*)calloc(num_bytes, sizeof(char));
	if (data == NULL) {
		// Handle memory allocation failure
		return NULL;
	}
	for (size_t i = 1; i <= num_bits; i++)
	{
		if (!IS_POWER_OF_TWO(i))
		{
			if (get_bit_value(encoded_data, i))
			{

				int index = get_adjusted_index(i, 1);
				data[(index - 1) / 8] |= 1 << (8 - (index) % 8);
				if (index % 8 == 0)
					data[(index - 1) / 8] |= 1;

			}
		}
	}
	return data;
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

void show(const char* file_name)
{
	long number_of_blocks = 0;
	ProtectionData* protection = read_protection_data_from_file(file_name, &number_of_blocks);
	if (protection == NULL)
		return NULL;
	BlockData* decoded_data = (BlockData*)malloc(number_of_blocks * sizeof(BlockData));

	for (size_t i = 0; i < number_of_blocks; i++)
	{
		char* block = original_data(&(protection[i]));
		int num_bytes = BLOCK_SIZE / 8;
		memcpy(decoded_data[i].data, block, num_bytes);

	}
	write_memory_to_file(remove_bin_extension(file_name), decoded_data, number_of_blocks * BLOCK_SIZE / 8);
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: error <file_name> <error_type> <error_place>\n");
		return 1;
	}

	const char* file_name = argv[1];
	if (!has_bin_extension(file_name)) {
		printf("not valid file. file must have bin extension\n");
		return 1;
	}
	show(file_name);
}