#pragma once

#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
#define BLOCK_SIZE 16
#define HAMMING1_SIZE 12
#define HAMMING2_SIZE 4



#pragma pack(push, 1)

typedef struct ProtectionData {
	unsigned char parityBit:2;
	char* hamming1;
	char* hamming2;
}ProtectionData;

#pragma pack(pop)

int parity_bit(const void * data, size_t len);
int calculate_number_of_parity_bits_for_block(int block_length);
int calculate_index_of_parity_bit(int parity);
int is_power_of_two(size_t num);
int get_adjusted_index(int index, int skipped);
int get_bit_value(const void* data, int bitPosition);
void binary_represent(void* data, int bits_number);
char* hamming2_encode(const void* data);
char* hamming1_encode(const void* data);
ProtectionData block_encode(void* block);
ProtectionData* encode(void* data, long length);




