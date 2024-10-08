#include "error.h"
#include <time.h>
#include <stdlib.h>
int random_in_range(int x, int y) {
	srand(time(NULL));
	return rand() % (y - x + 1) + x;
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


void single_bit(const char* file_name, int block)
{
	int place = random_in_range(1, HAMMING1_SIZE - 1);
	long number_of_blocks = 0;
	ProtectionData* protection = read_protection_data_from_file(file_name, &number_of_blocks);
	printf("block %d %d\n", number_of_blocks, block);
	if (block >= number_of_blocks)
	{
		printf("block %d %d\n", number_of_blocks, block);
		return;
	}
	if (protection == NULL)
	{
		printf("null\n");
		return;
	}
	change_bit_in_data(protection[block].hamming1, place);
	printf("bit flip found\n");
	write_protection_data_to_file(file_name, protection, number_of_blocks);
}

void pair_of_bits(const char* file_name, int block)
{
	int place = random_in_range(1, HAMMING1_SIZE - 2);
	long number_of_blocks = 0;
	ProtectionData* protection = read_protection_data_from_file(file_name, &number_of_blocks);
	if (block >= number_of_blocks)
	{
		printf("block %d %d\n", number_of_blocks, block);
		return;
	}
	if (protection == NULL)
	{
		printf("null\n");
		return;
	}
	change_bit_in_data(protection[block].hamming1, place);
	change_bit_in_data(protection[block].hamming1, place + 1);
	printf("two bits swapped found\n");
	write_protection_data_to_file(file_name, protection, number_of_blocks);
}

int main(int argc, char* argv[])
{
	if (argc != 4) {
		printf("Usage: error <file_name> <error_type> <error_place>\n");
		return 1;
	}

	const char* file_name = argv[1];
	const char* error_type = argv[2];
	int block = atoi(argv[3]);
	if (!has_bin_extension(file_name)) {
		printf("not valid file. file must have bin extension\n");
		return 1;
	}

	if (strcmp(error_type, "single_bit") == 1 && !strcmp(error_type, "two_bits") == 1) {
		printf("not valid parameter\n");
		return 1;
	}

	if (strcmp(error_type, "single_bit") == 0)
	{
		printf("single bit\n");
		single_bit(file_name, block);;
	}

	else
	{
		printf("two bits\n");
		pair_of_bits(file_name, block);
	}
}