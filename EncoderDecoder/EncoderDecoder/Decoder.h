#pragma once
#pragma once
#include "Encoder.h"
#include <stdio.h>  
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>


#pragma pack(push, 1)
typedef struct ProtectionData {
	unsigned char parityBit : 2;
	unsigned short dataWithHamming : HAMMING1_SIZE;
	unsigned char oddHamming : HAMMING2_SIZE;
}ProtectionData;
#pragma pack(pop)

unsigned int compare_between_original_and_existing_haming_1(unsigned int originalHamming, unsigned int existingHamming);
unsigned int compare_between_original_and_existing_haming_2(unsigned int originalHamming, unsigned int existingHamming);
void change_bit_in_data(char* block, unsigned int location);
int calculateThePositionOfHamming2ToHamming1(int position);
bool tryChangeBit(char* block, unsigned int parity, unsigned int location);
void twoBitSwapped(char* block, unsigned int differnceHaming, unsigned int differnceHamingOdd);
void blockDecoder(char* block, ProtectionData* protection);
void* decoder(ProtectionData* pd);


