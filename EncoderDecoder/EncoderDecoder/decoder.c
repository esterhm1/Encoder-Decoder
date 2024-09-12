#include "decoder.h"
int hamming1_result(char* encoded_data)
{
	int result = 0;
	int num_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	for (size_t i = 1; i <= num_bits; i++)
	{
		int this_parity_bit = 0;
		for (size_t j = (size_t)pow(2, i - 1) + 1; j <= BLOCK_SIZE + num_bits; j++) {
			if (j & (size_t)pow(2, i - 1))
			{
				this_parity_bit ^= get_bit_value(encoded_data, j);
			}
		}
		if (this_parity_bit != get_bit_value(encoded_data, (size_t)pow(2, i - 1)))
		{
			result |= 1 << (i - 1);
		}
	}
	return result;
}
int hamming2_result(char* data, MyType hamming)
{
	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block((BLOCK_SIZE + number_of_parity_bits_for_block) / 2);
	int result = 0;
	for (size_t i = 1; i <= number_of_parity_bits; i++)
	{
		int this_parity_bit = 0;
		for (int j = (int)pow(2, i - 1) + 1; j <= (BLOCK_SIZE + number_of_parity_bits_for_block) / 2 + number_of_parity_bits; j++) {
			int index = get_adjusted_index(j, 2);
			if (index != -1 && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}
		if (this_parity_bit != get_bit_value(&hamming, i))
		{
			result |= 1 << (i - 1);
		}
	}
	return get_adjusted_index(result, 2);
}
char* original_data(char* encoded_data)
{
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;
	size_t num_bytes = (num_bits + 7) / 8; // Round up to nearest byte
	char* data = (char*)calloc(num_bytes, sizeof(char));
	if (data == NULL) {
		// Handle memory allocation failure
		return NULL;
	}
	for (size_t i = 1; i <= num_bits; i++)
	{
		if (!IS_POWER_OF_TWO(i))
		{
			if (get_bit_value(encoded_data, i))
			{
				int index = get_adjusted_index(i, 1);
				data[(index - 1) / 8] |= 1 << (8 - (index) % 8);
				if (index % 8 == 0)
					data[(index - 1) / 8] |= 1;
			}
		}
	}
	return data;
}
void change_bit_in_data(char* data, int place)
{
	data[(place - 1) / 8] ^= (1 << ((8 - place) % 8));
}
int try_change_bit(char* data, int place)
{
	change_bit_in_data(data, place);
	return  hamming1_result(data);

}
void two_bits_swapped(char* encoded_data, MyType hamming2)
{
	int error = hamming2_result(encoded_data, hamming2);
	try_change_bit(encoded_data, error);
	if (try_change_bit(encoded_data, error - 1) == 0)
		return;
	try_change_bit(encoded_data, error - 1);
	try_change_bit(encoded_data, error + 1);
}
char* block_decoder(ProtectionData pd)
{
	if (((pd.parityBit & 1) ^ (pd.parityBit >> 1) & 1))
		return original_data(pd.hamming1);
	else
	{
		unsigned int error_place = hamming1_result(pd.hamming1);
		if (error_place == 0)
		{
			printf("valid\n");
			return original_data(pd.hamming1);
		}
		else
		{
			if (pd.parityBit != parity_bit_of_data(pd.hamming1, HAMMING1_SIZE))
			{

				printf("bit flip in %d\n", error_place);
				change_bit_in_data(pd.hamming1, error_place);
				return original_data(pd.hamming1);
			}
			else
			{
				printf("two bits swapped\n");
				two_bits_swapped(pd.hamming1, pd.hamming2);
			}

		}
	}
	return original_data(pd.hamming1);
}
char* decode(ProtectionData* pd, int len)
{
	int number_of_blocks = len / BLOCK_SIZE;
	if (len % BLOCK_SIZE)
		number_of_blocks++;
	char* decoded_data = (char*)malloc(len);
	char* data_ptr = decoded_data;
	if (pd == NULL) {
		fprintf(stderr, "Error: Null pointer encountered.\n");
		return decoded_data;
	}
	for (size_t i = 0; i < number_of_blocks; i++)
	{
		char* block = block_decoder(pd[i]);
		int num_bytes = BLOCK_SIZE / 8;
		memcpy(data_ptr, block, num_bytes);
		data_ptr += num_bytes;
	}
	return decoded_data;
}
