#pragma once
#include "encoder.h"
#include <stdbool.h>
#include <math.h>


int hamming1_result(char* encoded_data);
char* original_data(char* encoded_data);
void change_bit_in_data(char* data, int place);
void two_bits_swapped(char* encoded_data, char* hamming2);
int hamming2_result(char* data, char* hamming);
char* block_decoder(ProtectionData pd);
char* decode(ProtectionData* pd, int len);



