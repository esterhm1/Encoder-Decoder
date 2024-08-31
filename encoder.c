
#include "encoder.h"



int parity_bit(const void* data, size_t length) {
	int parity = 0;
	const unsigned char* byte_ptr = (const unsigned char*)data;
	size_t num_bytes = (length + 7) / 8;

	for (size_t i = 0; i < num_bytes; ++i) {
		unsigned char byte = byte_ptr[i];
		size_t bits_in_current_byte = (i == num_bytes - 1) ? (length % 8) : 8;
		for (size_t bit = 0; bit < bits_in_current_byte; ++bit) {
			uint8_t bit_value = (byte >> bit) & 1;
			parity ^= bit_value;
		}
	}

	if (parity == 1)
		return 3;
	return 0;
}

int calculate_index_of_parity_bit(int parity)
{
	int count = 0;
	while (parity != 1)
	{
		parity /= 2;
		count++;
	}
	return count;
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
	const unsigned char* byte_ptr = (const unsigned char*)data;

	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8); 
	if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1; 
	}

	unsigned char byte = byte_ptr[byteIndex];
	unsigned char mask = 1 << (bitIndex);
	int bitValue = (byte & mask) >> bitIndex;
	return bitValue;
}

char* hamming1_encode(char* data) {
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;
	size_t num_bytes = (num_bits + 7) / 8;

	char* encoded_data = (char*)calloc(num_bytes, sizeof(char));
	if (encoded_data == NULL) {
		return NULL;
	}

	for (size_t i = 1; i <= num_bits; i++) {
		if (is_power_of_two(i)) {
			int this_parity_bit = 0;
			for (size_t j = (size_t)pow(2, calculate_index_of_parity_bit(i)); j <= BLOCK_SIZE + number_of_parity_bits; j++) {
				int index = get_adjusted_index(j, 1);
				if (!is_power_of_two(j) && (j & (size_t)pow(2, calculate_index_of_parity_bit(i)))) {
					this_parity_bit ^= get_bit_value(data, index);
				}
			}

			if (this_parity_bit) {
				encoded_data[(i - 1) / 8] |= 1 << ((8 - i) % 8);
			}
		}
		else if (get_bit_value(data, get_adjusted_index(i, 1))) {
			encoded_data[(i - 1) / 8] |= 1 << ((8 - i) % 8);
		}
	}


	return encoded_data;
}


char* hamming2_encode(const void* data) {

	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	int number_of_parity_bits_for_hamming2 = calculate_number_of_parity_bits_for_block((BLOCK_SIZE+ number_of_parity_bits_for_block / 2));


	// Allocate memory for the result
	char* encoded_data = (char*)calloc((number_of_parity_bits_for_hamming2+7)/8, sizeof(char));
	if (encoded_data == NULL) {
		// Handle memory allocation failure
		return NULL;
	}

	for (int i = 1; i <= number_of_parity_bits_for_hamming2; i++) {
		int this_parity_bit = 0;
		for (int j = (int)pow(2, i - 1); j <= (BLOCK_SIZE+ number_of_parity_bits_for_block) / 2 + number_of_parity_bits_for_hamming2; j++) {
			int index = get_adjusted_index(j, 2);

			if (!is_power_of_two(j) && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}

		if (this_parity_bit) {
			encoded_data[(i - 1) / 8] |= 1 << ((8 - i) % 8);
		}
	}


	return encoded_data;
}



ProtectionData block_encode(void* block)
{
	ProtectionData protection;
	protection.hamming1 = hamming1_encode(block);
	protection.parityBit = parity_bit((protection.hamming1), HAMMING1_SIZE);
	protection.hamming2 = hamming2_encode(protection.hamming1);
	return protection;
}

ProtectionData* encode(char* data, long length)
{
 
	ProtectionData* pd = (ProtectionData*)malloc(length / BLOCK_SIZE * sizeof(ProtectionData));
	for (size_t i = 0; i < length/BLOCK_SIZE; i++)
	{
		pd[i] = block_encode(data);
		data += BLOCK_SIZE / 8;
	}
	return pd;
	
}



void binary_represent(void* data, int bits_number) {
	unsigned char* byte_ptr = (unsigned char*)data;

	int total_bytes = (bits_number + 7) / 8;

	for (int i = 0; i < total_bytes; ++i) {
		unsigned char byte = byte_ptr[i];

		int bits_to_print = (i == total_bytes - 1) ? (bits_number % 8) : 8;
		if (bits_to_print == 0) bits_to_print = 8; // אם זה byte הראשון

		for (int bit = bits_to_print - 1; bit >= 0; --bit) {
			printf("%d ", (byte >> bit) & 1);
		}
	}

	printf("\n");
}