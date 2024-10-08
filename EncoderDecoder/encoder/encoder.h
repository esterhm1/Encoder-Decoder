#pragma once

#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>


#define _CRT_SECURE_NO_WARNINGS
#define BLOCK_SIZE 128
#define HAMMING1_SIZE 136
#define HAMMING2_SIZE 4
#if BLOCK_SIZE < 256
typedef unsigned char MyType;
#elif BLOCK_SIZE < 65536
typedef unsigned short MyType;
#else
typedef unsigned int MyType;
#endif

#define NUM_BYTES(length) (((length) + 7) / 8)
#define IS_POWER_OF_TWO(num) (((num) & ((num) - 1)) == 0 && (num) != 0)
#define EQUALS_TO_PARITY_BIT(a, b) ((a) & (b) ? 1 : 0)

#pragma pack(push, 1)

typedef struct ProtectionData {
	char hamming1[(HAMMING1_SIZE + 7) / 8];
	MyType parityBit : 2;
	MyType hamming2;
}ProtectionData;

#pragma pack(pop)

typedef struct BlockData {
	char data[BLOCK_SIZE / 8];
}BlockData;


//functions
char* read_file_to_memory(const char* filename, unsigned long long int* out_length);
void write_protection_data_to_file(const char* file_name, ProtectionData* protection, int final_blocks_num);
char* add_bin_extension(const char* file_path);
int has_bin_extension(const char* filename);
void binary_represent(void* data, int bits_number);

ProtectionData* read_protection_data_from_file(const char* file_name, long* number_of_blocks);
char* remove_bin_extension(const char* file_path);
int write_memory_to_file(const char* filename, const char* data, unsigned long long int length);

int get_adjusted_index(int index, int skipped);
int get_bit_value(char* data, int bitPosition);
int calculate_number_of_parity_bits_for_block(int);

void hamming1_encode(char* data, int index);
MyType hamming2_encode(const void* data);
MyType parity_bit_of_data(char* data, size_t length);

int hamming1_result(char* encoded_data);
int hamming2_result(char* data, MyType hamming);
char* original_data(char* encoded_data);
void change_bit_in_data(char* data, int place);
void two_bits_swapped(char* encoded_data, MyType hamming2);
int try_change_bit(char* data, int place);

void block_encode(void* block, int index);
void encode(char* data, long length, char* bin_file_name);
long encode_files(const char* file_name);


char* block_decoder(ProtectionData pd);
char* decode(ProtectionData* protection, int number_of_blocks);
void decode_files(const char* file_name);

int main(int argc, char* argv[]);

ProtectionData* pd;



