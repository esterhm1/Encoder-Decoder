#include "Decoder.h"
#define DATA_SIZE 100
#define BLOCK_SIZE_WITH_HAMMING 12
#define HAMMING1_SIZE 4

void* extract_data(char* dataWithHamming) {
	
}

void correct_one_bit_flip(char* dataWithHamming, int place) {
	(*dataWithHamming) ^= ((int)pow(2, (BLOCK_SIZE_WITH_HAMMING - place)));
}
bool getBit(char* bits, int index) {
	return (!!((*bits) >> index & 1));
}
void Change_bit_for_all_Parity_bits(unsigned int* parityBits, int location) {

	for (int i = 0; i < HAMMING1_SIZE; i++) {
		if ((1 << i) & location) {
			//update the parity bit after changing
			(*parityBits) ^= 1 << i;
		}
	}

}
unsigned int compare_between_original_and_existing_haming_1(char* dataWithHamming, char* existingHamming) {
	unsigned int difference = (!(getBit(dataWithHamming, 0) == getBit(existingHamming, 0)) << (HAMMING1_SIZE - 1));
	for (int i = 1; i < BLOCK_SIZE_WITH_HAMMING; i *= 2) {
		//update the parity bit the dataWithHamming difference
		difference ^= (!(getBit(dataWithHamming, i) == getBit(existingHamming, i)) << (HAMMING1_SIZE - i - 1));
	}
	return difference;
}
unsigned int compare_between_original_and_existing_haming_2(char* originalHamming, char* existingHamming) {
	unsigned int difference = 0;
	for (int i = 0; i < HAMMING2_SIZE; i++) {
		//update the parity bit the specific difference
		difference ^= (!(getBit(originalHamming,i) == getBit(existingHamming, i)) << (HAMMING2_SIZE - i - 1));
	}
	return difference;
}
bool tryChangeBit(char* dataWithHamming, unsigned int* differnceHamingWithData, unsigned int location) {
	//so that there is no exception
	if (location > 0) {
		Change_bit_for_all_Parity_bits(differnceHamingWithData, location);
		//if is the adjacent bit that was replaced with the resulting location
		if ((*differnceHamingWithData) == 0) {
			//update the original data
			correct_one_bit_flip(dataWithHamming, location);
			return true;
		}
	}
	return false;
}
void twoBitSwapped(char* dataWithHamming, unsigned int* differnceHamingWithData, unsigned int differnceHamingOdd) {

	if (!tryChangeBit(dataWithHamming, differnceHamingWithData, differnceHamingOdd + 1)) {
		tryChangeBit(dataWithHamming, differnceHamingWithData, differnceHamingOdd - 1);
	}
}
void correct_two_bit_flip(char* data,ProtectionData* protection_struct, unsigned int* differnceHamingWithData) {
	char* existingHamming = hamming_encode(data, 2);
	unsigned int difference = compare_between_original_and_existing_haming_2(protection_struct->oddHamming, existingHamming);
	difference = calculate_the_position_of_Hamming2_to_Hamming1(difference);
	twoBitSwapped(protection_struct->dataWithHamming, differnceHamingWithData, difference);
}

unsigned int hamming_decoder(char* data,char* dataWithHamming) {
	char* existingHamming = hamming_encode(data, 1);
	return compare_between_original_and_existing_haming_1(dataWithHamming, existingHamming);
}

void* block_decoder(ProtectionData* protection_struct) { 
	// The parity bits aren't the same ---> if((int)protection_struct.parityBit != 3 && (int)protection_struct.parityBit != 0) 
	if (((*protection_struct).parityBit & 1) ^ (((*protection_struct).parityBit >> 1) & 1)) {
		return extract_data((*protection_struct).dataWithHamming);
	}
	char* data = extract_data(protection_struct->dataWithHamming);
	// the function hamming_decoder only find the place that may be an error
	unsigned int error_place = hamming_decoder(data,protection_struct->dataWithHamming);

	if (error_place == 0) { // There is no error
		return data; // הפונקציה הזו מחלצת את המידע  
	}

	// now, there is an error and the two parityBit are the same!!!
	// if the XOR on dataWithHamming is not the same as the first/ second (same) in other words it's equal to 1 - there is one bit flip
	if (MultyXor((*protection_struct).dataWithHamming) != (*protection_struct).parityBit & 1) {
		correct_one_bit_flip((*protection_struct).dataWithHamming, error_place);
	}
	else {
		correct_two_bit_flip(data,protection_struct,error_place);
	}

	return extract_data(protection_struct->dataWithHamming);
}

void* decoder(ProtectionData* pd) {
	int index = 0;
	int num_of_blocks = DATA_SIZE / BLOCK_SIZE_WITH_HAMMING;
	while (index < num_of_blocks) {
		// keep??
		block_decoder(&pd);
		pd += BLOCK_SIZE_WITH_HAMMING;
		index += 1;
	}

}