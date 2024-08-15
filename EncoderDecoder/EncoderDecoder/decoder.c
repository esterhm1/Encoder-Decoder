#include "Decoder.h"
#define DATA_SIZE 100
#define BLOCK_SIZE_WITH_HAMMING 12

int calculate_amount_of_parity_bits(int size) {
	int count = 0;
	int power = 1;

	while (power <= size) {
		count++;
		power *= 2;
	}

	return count;
}


void* extract_data(char* data_with_hamming) {
	int parity_bits = calculate_amount_of_parity_bits(BLOCK_SIZE_WITH_HAMMING);
	int only_data_size = BLOCK_SIZE_WITH_HAMMING - parity_bits;
	char* only_data = (char*)malloc(only_data_size * sizeof(char));

	if (only_data == NULL) {
		return NULL;
	}

	int index_data_with_hamming = 1, index_only_data = 0;

	while (index_data_with_hamming < BLOCK_SIZE_WITH_HAMMING) {
		// if not power of two add the bit.
		if ((index_data_with_hamming & (index_data_with_hamming - 1)) != 0) {
			only_data[index_only_data] = data_with_hamming[index_data_with_hamming];
			index_only_data++;
		}
		index_data_with_hamming++;
	}

	return (void*)only_data;
}

void correct_one_bit_flip(char* dataWithHamming, int place) {

}

void correct_two_bit_flip(ProtectionData* protection_struct) {

}

int hamming_decoder(char* dataWithHamming) {

}

void* block_decoder(ProtectionData* protection_struct) { 
	// The parity bits aren't the same ---> if((int)protection_struct.parityBit != 3 && (int)protection_struct.parityBit != 0) 
	if (((*protection_struct).parityBit & 1) ^ (((*protection_struct).parityBit >> 1) & 1)) {
		return extract_data((*protection_struct).dataWithHamming);
	}

	// the function hamming_decoder only find the place that may be an error
	int error_place = hamming_decoder((*protection_struct).dataWithHamming);

	if (error_place == 0) { // There is no error
		return extract_data((*protection_struct).dataWithHamming); // הפונקציה הזו מחלצת את המידע  
	}

	// now, there is an error and the two parityBit are the same!!!
	// if the XOR on dataWithHamming is not the same as the first/ second (same) in other words it's equal to 1 - there is one bit flip
	if (MultyXor((*protection_struct).dataWithHamming) != (*protection_struct).parityBit & 1) {
		correct_one_bit_flip((*protection_struct).dataWithHamming, error_place);
	}
	else {
		correct_two_bit_flip(protection_struct);
	}

	return extract_data((*protection_struct).dataWithHamming);
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