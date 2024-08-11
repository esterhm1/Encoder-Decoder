#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
#define BLOCK_SIZE 8



#pragma pack(push, 1)

typedef struct ProtectionData {
	unsigned char parityBit;
	char* hamming1;
	char* hamming2;
}ProtectionData;

#pragma pack(pop)



int MultyXor(void* data, size_t len) {
	unsigned char* byte_ptr = (unsigned char*)data;
	int parity = 0;

	for (size_t i = 0; i < len; ++i) {
		unsigned char byte = byte_ptr[i];

		for (int bit = 0; bit < 8; ++bit) {

			uint8_t bit_value = (byte >> bit) & 1;

			parity ^= bit_value;
		}
	}
	return parity;
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
	// אם האינדקס הוא חזקה של 2, החזר -1
	if (is_power_of_two(index)) {
		return -1;
	}

	// אחרת, חשב את האינדקס המתואם
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
	// המרת המצביע ל-unsigned char* כדי לגשת לנתונים כבתים
	const unsigned char* byte_ptr = (const unsigned char*)data;

	// חישוב מיקום ה-byte והביט בתוך ה-byte
	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8); // 7 - bitIndex כדי לקרוא ביטים משמאל לימין

	// נוודא שמיקום ה-byte הוא בתוך הגבולות של האזור הזיכרון
	// (נניח שהאזור זיכרון הוא קטן ממערך של unsigned char)
	if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1; // שגיאה: מיקום מחוץ לטווח
	}

	// הוצאת הבית המתאים מהמצביע
	unsigned char byte = byte_ptr[byteIndex];

	// יצירת מסכה לבדוק את ערך הביט המתאים
	unsigned char mask = 1 << (bitIndex);

	// שימוש במסכה כדי לבדוק את ערך הביט המתאים
	int bitValue = (byte & mask) >> bitIndex;

	return bitValue;
}

void binary_represent(void* data, int bits_number) {
	// מצביע לנתונים כ-unsigned char
	unsigned char* byte_ptr = (unsigned char*)data;

	// חישוב מספר הבתים הכולל
	int total_bytes = (bits_number + 7) / 8; // Round up to include all bytes

	// מדפיס את הסיביות
	for (int i = 0; i < total_bytes; ++i) {
		unsigned char byte = byte_ptr[i];

		// נוודא שאנחנו לא מדפיסים יותר סיביות מהנתון
		int bits_to_print = (i == total_bytes - 1) ? (bits_number % 8) : 8;
		if (bits_to_print == 0) bits_to_print = 8; // אם זה byte הראשון

		for (int bit = bits_to_print - 1; bit >= 0; --bit) {
			printf("%d ", (byte >> bit) & 1);
		}
	}

	printf("\n");
}


char* hamming_encode(const void* data, size_t len, int skipped) {
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(len / skipped);

	// Allocate memory for storing parity bits
	unsigned char* buffer = (unsigned char*)malloc((number_of_parity_bits + 7) / 8);
	if (buffer == NULL) {
		return NULL; // Handle allocation failure
	}

	// Initialize buffer to zero
	for (int i = 0; i < (number_of_parity_bits + 7) / 8; i++) {
		buffer[i] = 0;
	}

	// Compute and store the parity bits
	for (int i = 1; i <= number_of_parity_bits; i++) {
		int this_parity_bit = 0;

		for (int j = (int)pow(2, i - 1); j <= len / skipped + number_of_parity_bits; j++) {
			int index = get_adjusted_index(j, skipped);

			if (index != -1 && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}

		// Store the parity bit in the buffer
		if (this_parity_bit) {
			buffer[(number_of_parity_bits - i) / 8] |= (1 << ((number_of_parity_bits - i) % 8));
		}
	}

	return buffer;
}


void block_encode(void* block)
{
	ProtectionData protection;
	protection.parityBit = MultyXor(block, BLOCK_SIZE);
	protection.hamming1 = hamming_encode(block, BLOCK_SIZE, 1);
	protection.hamming2 = hamming_encode(block, BLOCK_SIZE, 2);
	FILE* file = NULL;
	fopen_s(&file, "output.bin", "wb");
	fwrite(&protection, sizeof(protection), 1, file);
	fclose(file);
}


int main()
{
	int data = 3;
	block_encode(&data);
	printf("%d\n", sizeof(ProtectionData));
	read_protection_data();
}