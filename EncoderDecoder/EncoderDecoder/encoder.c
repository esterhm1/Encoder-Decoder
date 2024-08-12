#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
#define BLOCK_SIZE 8
#define HAMMING1_SIZE 4
#define HAMMING2_SIZE 3



#pragma pack(push, 1)

typedef struct ProtectionData {
	unsigned char parityBit : 1;
	unsigned int hamming1 : HAMMING1_SIZE;
	unsigned int hamming2 : HAMMING2_SIZE;
}ProtectionData;

#pragma pack(pop)



int MultyXor(void* data, size_t len) {
	unsigned char* byte_ptr = (unsigned char*)data;
	int parity = 0;

	for (size_t i = 0; i < len; ++i) {
		unsigned char byte = byte_ptr[i];

		for (int bit = 0; bit < 8; ++bit) {

			uint8_t bit_value = (byte >> bit) & 1;

			parity ^= bit_value;
		}
	}
	return parity;
}

int calculate_number_of_parity_bits_for_block(int block_length)
{
	int count = 0;
	while ((1 << count) < (block_length + count + 1)) {
		count++;
	}

	return count;
}

int is_power_of_two(size_t num) {
	return (num & (num - 1)) == 0;
}

int get_adjusted_index(int index, int skipped) {
	// �� ������� ��� ���� �� 2, ���� -1
	if (is_power_of_two(index)) {
		return -1;
	}

	// ����, ��� �� ������� ������
	int adjusted_index = 1;
	for (size_t i = 1; i < index; ++i) {
		if (!is_power_of_two(i)) {
			++adjusted_index;
		}
	}
	if (skipped == 1)
		return adjusted_index;
	return adjusted_index * 2 - 1;
}

int get_bit_value(const void* data, int bitPosition) {
	bitPosition--;
	// ���� ������ �-unsigned char* ��� ���� ������� �����
	const unsigned char* byte_ptr = (const unsigned char*)data;

	// ����� ����� �-byte ����� ���� �-byte
	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8); // 7 - bitIndex ��� ����� ����� ����� �����

	// ����� ������ �-byte ��� ���� ������� �� ����� �������
	// (���� ������ ������ ��� ��� ����� �� unsigned char)
	if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1; // �����: ����� ���� �����
	}

	// ����� ���� ������ �������
	unsigned char byte = byte_ptr[byteIndex];

	// ����� ���� ����� �� ��� ���� ������
	unsigned char mask = 1 << (bitIndex);

	// ����� ����� ��� ����� �� ��� ���� ������
	int bitValue = (byte & mask) >> bitIndex;

	return bitValue;
}

void binary_represent(void* data, int bits_number) {
	// ����� ������� �-unsigned char
	unsigned char* byte_ptr = (unsigned char*)data;

	// ����� ���� ����� �����
	int total_bytes = (bits_number + 7) / 8; // Round up to include all bytes

	// ����� �� �������
	for (int i = 0; i < total_bytes; ++i) {
		unsigned char byte = byte_ptr[i];

		// ����� ������ �� ������� ���� ������ ������
		int bits_to_print = (i == total_bytes - 1) ? (bits_number % 8) : 8;
		if (bits_to_print == 0) bits_to_print = 8; // �� �� byte ������

		for (int bit = bits_to_print - 1; bit >= 0; --bit) {
			printf("%d ", (byte >> bit) & 1);
		}
	}

	printf("\n");
}

void print_binary(unsigned int num) {
	const int BITS = sizeof(num) * 8;  // ���� ������ �-unsigned int

	// ����� ������ ������ �� �� ������ ���� ���� ����� �����
	for (int i = BITS - 1; i >= 0; --i) {
		// ���� �� ���� ������
		printf("%d", (num >> i) & 1);

		// ���� ���� ���� �� 8 �����
		if (i % 8 == 0 && i != 0) {
			printf(" ");
		}
	}

	// ���� ���� ���� ����
	printf("\n");
}


uint64_t hamming_encode(const void* data, size_t len, int skipped) {
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(len / skipped);
	uint64_t result = 0;

	// Compute and store the parity bits
	for (int i = 1; i <= number_of_parity_bits; i++) {
		int this_parity_bit = 0;

		for (int j = (int)pow(2, i - 1); j <= len / skipped + number_of_parity_bits; j++) {
			int index = get_adjusted_index(j, skipped);

			if (index != -1 && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}

		// Store the parity bit in the result
		if (this_parity_bit) {
			result |= (1ULL << (number_of_parity_bits - i));
		}
	}

	return result;
}


void block_encode(void* block, char* protection_file_name)
{
	ProtectionData protection;
	protection.parityBit = MultyXor(block, BLOCK_SIZE);
	protection.hamming1 = hamming_encode(block, BLOCK_SIZE, 1);
	protection.hamming2 = hamming_encode(block, BLOCK_SIZE, 2);
	FILE* file = NULL;
	fopen_s(&file, protection_file_name, "ab");
	fwrite(&protection, sizeof(protection), 1, file);
	fclose(file);
}

void encode(void* data, long length)
{
	char* protection_file_name = "output.bin";
	void* ptr;
	int index = 0;
	while (index + BLOCK_SIZE <= length)
	{
		block_encode(data, protection_file_name);
		index += BLOCK_SIZE;
	}



}

void read_protection_data() {
	FILE* file = NULL;
	fopen_s(&file, "output.bin", "rb"); // Read mode
	if (file == NULL) {
		perror("Error opening file");
		return;
	}

	fseek(file, 0, SEEK_END); // Move to end of file to get size
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET); // Move back to start of file

	while (ftell(file) < file_size) {
		ProtectionData d;
		fread(&d, sizeof(ProtectionData), 1, file);
		printf("%d\n", d.parityBit);
		print_binary(d.hamming1);
		print_binary(d.hamming2);

	}

	fclose(file);
}



int main()
{
	int data = 3;
	encode(&data, sizeof(data) * 8);


	//�� ����� ���� ���� �����
	read_protection_data();
}