#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "time.h"



extern "C" {
#include "decoder.h"
#include "files.h"
}

char* generate_random_buffer(size_t length, size_t* actual_length) {
	// Allocate memory for the buffer
	char* buffer = (char*)malloc(length * sizeof(char));
	if (buffer == NULL) {
		printf("Memory allocation failed.\n");
		return NULL;
	}

	// Seed the random number generator
	srand(time(NULL));

	// Fill the buffer with random characters
	for (size_t i = 0; i < length; i++) {
		buffer[i] = rand() % 256; // Generate random byte
	}

	// Return the actual length
	*actual_length = length;

	// Return the pointer to the buffer
	return buffer;
}


int compare_memory(const char* ptr1, const char* ptr2, unsigned long long int num) {

	for (size_t i = 0; i < num; i++) {
		if (ptr1[i] != ptr2[i]) {
			return 0;
		}
	}
	return 1;
}

int random_in_range(int x, int y) {
	srand(time(NULL));
	return rand() % (y - x + 1) + x;
}


TEST_CASE("valid data") {
	unsigned long long int data_length = 0;
	char* data = read_file_to_string("file.txt", &data_length);
	encode(data, data_length * 8);
	int len = data_length * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	char* decoded = decode("output.bin", data_length * 8);
	CHECK(compare_memory(data, decoded, data_length) == 1);
}


TEST_CASE("data with bit flip") {
	int place = random_in_range(1, HAMMING1_SIZE - 1);
	unsigned long long int data_length = 0;
	char* data = read_file_to_string("file.txt", &data_length);
	encode(data, data_length * 8);
	int len = data_length * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	ProtectionData* protection = read_protection_data_from_file("output.bin", length);
	change_bit_in_data(protection[0].hamming1, place);
	write_protection_data_to_file("output.bin", protection, length);
	char* decoded = decode("output.bin", data_length * 8);
	CHECK(compare_memory(data, decoded, data_length) == 1);
}

//3
TEST_CASE("2 bits swapped in data") {
	int place = random_in_range(1, HAMMING1_SIZE - 2);
	unsigned long long int data_length = 0;
	char* data = read_file_to_string("file.txt", &data_length);
	encode(data, data_length * 8);
	int len = data_length * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	ProtectionData* protection = read_protection_data_from_file("output.bin", length);
	change_bit_in_data(protection[0].hamming1, place);
	change_bit_in_data(protection[0].hamming1, place + 1);
	write_protection_data_to_file("output.bin", protection, length);
	char* decoded = decode("output.bin", data_length * 8);
	CHECK(compare_memory((char*)data, decoded, data_length) == 1);
}

//4
TEST_CASE("error in parity bit of data") {
	unsigned long long int data_length = 0;
	char* data = read_file_to_string("file.txt", &data_length);
	encode(data, data_length * 8);
	int len = data_length * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	ProtectionData* protection = read_protection_data_from_file("output.bin", length);
	protection[0].parityBit = random_in_range(1, 2);
	write_protection_data_to_file("output.bin", protection, length);
	char* decoded = decode("output.bin", data_length * 8);
	CHECK(compare_memory(data, decoded, data_length) == 1);

}

//5
TEST_CASE("the block is smaller than the block size") {
	char data[2] = { 'a','b' };
	encode(&data, sizeof(data) * 8);
	int len = sizeof(data) * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	ProtectionData* protection = read_protection_data_from_file("output.bin", length);
	write_protection_data_to_file("output.bin", protection, length);
	char* decoded = decode("output.bin", sizeof(data) * 8);
	CHECK(compare_memory((char*)data, decoded, sizeof(data)) == 1);

}

//6
//TEST_CASE("data is null") {
//	char* data = NULL;
//	encode(data, sizeof(data)*8);
//	char* decoded = decode(sizeof(data) * 8);
//	CHECK(decoded==data);
//}

//7
TEST_CASE("bit from parity bit was swapped with bit from hamming1") {
	int place = random_in_range(1, HAMMING1_SIZE - 1);
	unsigned long long int data_length = 0;
	char* data = read_file_to_string("file.txt", &data_length);
	encode(data, data_length * 8);
	int len = data_length * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	ProtectionData* protection = read_protection_data_from_file("output.bin", length);
	protection[0].parityBit = pd[0].parityBit == 0 ? 2 : 1;
	change_bit_in_data(protection[0].hamming1, HAMMING1_SIZE);	
	write_protection_data_to_file("output.bin", protection, length);
	char* decoded = decode("output.bin", data_length * 8);
	CHECK(compare_memory(data, decoded, data_length) == 1);
}

//8
TEST_CASE("bit flip in last place of byte") {
	int place = random_in_range(1, HAMMING1_SIZE / 8);
	short data = 67;
	encode(&data, sizeof(data) * 8);
	int len = sizeof(data) * 8;
	int length = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		length++;
	ProtectionData* protection = read_protection_data_from_file("output.bin", length);
	change_bit_in_data(protection[0].hamming1, place * 8);
	write_protection_data_to_file("output.bin", protection, length);
	char* decoded = decode("output.bin", sizeof(data) * 8);
	CHECK(compare_memory((char*)&data, decoded, sizeof(data)) == 1);
}

TEST_CASE("random errors") {
	int place = random_in_range(1, HAMMING1_SIZE / 8);
	short data = 67;
	encode(&data, sizeof(data) * 8);
	change_bit_in_data(pd[0].hamming1, place * 8);
	char* decoded = decode("output.bin", sizeof(data) * 8);
	CHECK(compare_memory((char*)&data, decoded, sizeof(data)) == 1);
}




