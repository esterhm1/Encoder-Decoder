#include "encoder.h"
//calculate the parity bit of the block (Multy_Xor)
MyType parity_bit_of_data(char* data, size_t length) {
	int parity = 0;
	for (size_t i = 0; i < NUM_BYTES(length); ++i) {
		unsigned char byte = data[i];
		size_t bits_in_current_byte = (i == NUM_BYTES(length) - 1) ? (length % 8) : 8;
		for (size_t bit = 0; bit < bits_in_current_byte; ++bit) {
			uint8_t bit_value = (byte >> bit) & 1;
			parity ^= bit_value;
		}
	}
	if (parity == 1)
		return 3;
	return 0;
}
int calculate_number_of_parity_bits_for_block()
{
	int count = 0;
	while ((1 << count) < (BLOCK_SIZE + count + 1)) {
		count++;
	}
	return count;
}
int get_adjusted_index(int index, int skipped) {
	int adjusted_index = 1;
	for (size_t i = 1; i < index; ++i) {
		if (!IS_POWER_OF_TWO(i)) {
			++adjusted_index;
		}
	}
	if (skipped == 1)
		return adjusted_index;
	return adjusted_index * 2 - 1;
}
int get_bit_value(char* data, int bitPosition) {
	bitPosition--;
	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8);
	if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1;
	}
	unsigned char byte = data[byteIndex];
	unsigned char mask = 1 << (bitIndex);
	int bitValue = (byte & mask) >> bitIndex;
	return bitValue;
}
void hamming1_encode(char* data, int index) {
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;
	//fill the array of hamming1 with zeros
	memset(pd[index].hamming1, 0, sizeof(pd[index].hamming1));
	//go through all the bits in the array
	for (size_t i = 1; i <= num_bits; i++) {
		//if its a parity bit
		if (IS_POWER_OF_TWO(i)) {
			int this_parity_bit = 0;
			//go through all bits from the current bit onwards
			for (size_t j = i + 1; j <= BLOCK_SIZE + number_of_parity_bits; j++) {
				int index = get_adjusted_index(j, 1);
				//check if the current bit should be calculated as a part of this parity bit
				if (EQUALS_TO_PARITY_BIT(i, j)) {
					this_parity_bit ^= get_bit_value(data, index);
				}
			}
			//save the parity bit value in the appropriate place
			if (this_parity_bit) {
				(pd[index].hamming1)[(i - 1) / 8] |= 1 << ((8 - i) % 8);
			}
		}
		//if its not a parity bit
		else
		{
			//save the bit value in the appropriate place
			if (get_bit_value(data, get_adjusted_index(i, 1))) {
				pd[index].hamming1[(i - 1) / 8] |= 1 << ((8 - i) % 8);
			}
		}
	}
}
MyType hamming2_encode(const void* data) {
	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	int number_of_parity_bits_for_hamming2 = calculate_number_of_parity_bits_for_block((BLOCK_SIZE + number_of_parity_bits_for_block) / 2);
	// Initialize the result with zero
	MyType encoded_data = 0;
	//go through all the parity bits
	for (int i = 1; i <= number_of_parity_bits_for_hamming2; i++) {
		int this_parity_bit = 0;
		//go through all the bits from the current parity bits and on
		for (int j = (int)pow(2, i - 1); j <= (BLOCK_SIZE + number_of_parity_bits_for_block) / 2 + number_of_parity_bits_for_hamming2; j++) {
			int index = get_adjusted_index(j, 2);
			//check if the current bit should be calculated as a part of this parity bit
			if (!IS_POWER_OF_TWO(j) && EQUALS_TO_PARITY_BIT(j, (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}
		if (this_parity_bit) {
			encoded_data |= (MyType)(1 << ((8 * sizeof(MyType) - i) % (8 * sizeof(MyType))));
		}
	}
	return encoded_data;
}
void block_encode(void* block, int index)
{
	hamming1_encode(block, index);
	pd[index].parityBit = parity_bit_of_data((pd[index].hamming1), HAMMING1_SIZE);
	pd[index].hamming2 = hamming2_encode(pd[index].hamming1);
}
void encode(char* data, long length) {
	// חישוב מספר הבלוקים המלאים
	size_t full_blocks = length / BLOCK_SIZE;
	// חישוב הגודל של הבלוק האחרון אם הוא חלקי
	size_t last_block_size = length % BLOCK_SIZE;
	// הקצה זיכרון ל-ProtectionData עבור כל הבלוקים המלאים
	pd = (ProtectionData*)malloc((full_blocks + 1) * sizeof(ProtectionData));
	if (pd == NULL) {
		perror("Error allocating memory");
		return;
	}
	// קידוד בלוקים מלאים
	for (size_t i = 0; i < full_blocks; i++) {
		block_encode(data, i);
		data += BLOCK_SIZE / 8;
	}
	// טיפול בבלוק האחרון אם הוא חלקי
	if (last_block_size > 0) {
		// יצירת בלוק חדש בגודל BLOCK_SIZE
		char* last_block = (char*)malloc((BLOCK_SIZE / 8));
		if (last_block == NULL) {
			perror("Error allocating memory for last block");
			free(pd);
			return;
		}
		// אתחול יתרת הבלוק לאפסים
		memset(last_block, 0, NUM_BYTES(BLOCK_SIZE));
		// העתקת הבלוק האחרון מהנתונים
		memcpy(last_block, data, NUM_BYTES(last_block_size));
		// קידוד הבלוק האחרון
		block_encode(last_block, full_blocks);
		// ניקוי
		free(last_block);
	}
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