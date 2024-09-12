#pragma once
#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define _CRT_SECURE_NO_WARNINGS
#define BLOCK_SIZE 16
#define HAMMING1_SIZE 22
#define HAMMING2_SIZE 6
#if BLOCK_SIZE < 256
typedef unsigned char MyType;
#elif BLOCK_SIZE < 65536
typedef unsigned short MyType;
#else
typedef unsigned int MyType;
#endif
#define NUMBER_OF_PARITY_BITS_FOR_BLOCK(num_bits) \
    ( \
        ( \
            ( \
                (num_bits) < 1 ? 0 : \
                ((1 << ((int)log2(num_bits + 1) + 1)) - (num_bits) - 1) < 0 ? \
                (int)log2(num_bits + 1) + 1 : \
                (int)log2(num_bits + 1) \
            ) \
        ) \
    )
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
MyType parity_bit_of_data(const void* data, size_t len);
int calculate_number_of_parity_bits_for_block(int);
int calculate_index_of_parity_bit(int parity);
int is_power_of_two(size_t num);
int get_adjusted_index(int index, int skipped);
int get_bit_value(const void* data, int bitPosition);
void binary_represent(void* data, int bits_number);
MyType hamming2_encode(const void* data);
void hamming1_encode(const void* data, int index);
void block_encode(void* block, int index);
void encode(void* data, long length);
ProtectionData* pd;
