#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
#define BLOCK_SIZE 8
#define HAMMING1_SIZE 4
#define HAMMING2_SIZE 3



#pragma pack(push, 1)

typedef struct ProtectionData {
	unsigned char parityBit : 1;
	unsigned int hamming1 : HAMMING1_SIZE;
	unsigned int hamming2 : HAMMING2_SIZE;
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

void print_binary(unsigned int num) {
	const int BITS = sizeof(num) * 8;  // מספר הביטים ב-unsigned int

	// נשתמש בלולאה להדפיס את כל הביטים מהכי גבוה לנמוך ביותר
	for (int i = BITS - 1; i >= 0; --i) {
		// הדפס את הביט המתאים
		printf("%d", (num >> i) & 1);

		// הוסף רווח אחרי כל 8 ביטים
		if (i % 8 == 0 && i != 0) {
			printf(" ");
		}
	}

	// הדפס שורה חדשה בסוף
	printf("\n");
}


uint64_t hamming_encode(const void* data, size_t len, int skipped) {
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(len / skipped);
	uint64_t result = 0;

	// Compute and store the parity bits
	for (int i = 1; i <= number_of_parity_bits; i++) {
		int this_parity_bit = 0;

		for (int j = (int)pow(2, i - 1); j <= len / skipped + number_of_parity_bits; j++) {
			int index = get_adjusted_index(j, skipped);

			if (index != -1 && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}

		// Store the parity bit in the result
		if (this_parity_bit) {
			result |= (1ULL << (number_of_parity_bits - i));
		}
	}

	return result;
}


void block_encode(void* block, char* protection_file_name)
{
	ProtectionData protection;
	protection.parityBit = MultyXor(block, BLOCK_SIZE);
	protection.hamming1 = hamming_encode(block, BLOCK_SIZE, 1);
	protection.hamming2 = hamming_encode(block, BLOCK_SIZE, 2);
	FILE* file = NULL;
	fopen_s(&file, protection_file_name, "ab");
	fwrite(&protection, sizeof(protection), 1, file);
	fclose(file);
}

void encode(void* data, long length)
{
	char* protection_file_name = "output.bin";
	void* ptr;
	int index = 0;
	while (index + BLOCK_SIZE <= length)
	{
		block_encode(data, protection_file_name);
		index += BLOCK_SIZE;
	}



}

void read_protection_data() {
	FILE* file = NULL;
	fopen_s(&file, "output.bin", "rb"); // Read mode
	if (file == NULL) {
		perror("Error opening file");
		return;
	}

	fseek(file, 0, SEEK_END); // Move to end of file to get size
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET); // Move back to start of file

	while (ftell(file) < file_size) {
		ProtectionData d;
		fread(&d, sizeof(ProtectionData), 1, file);
		printf("%d\n", d.parityBit);
		print_binary(d.hamming1);
		print_binary(d.hamming2);

	}

	fclose(file);
}

#include <stdbool.h>
unsigned int compare_between_original_and_existing_haming_1(uint64_t originalHamming, unsigned int existingHamming) {
	unsigned int difference = 0;
	for (int i = 0; i < HAMMING1_SIZE; i++) {
		//update the parity bit the specific difference
        difference ^= (!(!!(originalHamming >> i & 1) == !!(existingHamming >> i & 1)) << (HAMMING1_SIZE - i-1));
	}
	return difference;
}
unsigned int compare_between_original_and_existing_haming_2(uint64_t originalHamming, unsigned int existingHamming) {
	unsigned int difference = 0;
	for (int i = 0; i < HAMMING2_SIZE; i++) {
		//update the parity bit the specific difference
		difference ^= (!(!!(originalHamming >> i & 1) == !!(existingHamming >> i & 1)) << (HAMMING2_SIZE - i-1));
	}
	return difference;
}
void change_bit_in_data(char* block, unsigned int location) {
	(*block) ^= ((int)pow(2, BLOCK_SIZE - (location - (int)log2(location) - 1)));
}

void Change_bit_for_all_Parity_bits(unsigned int* parityBits, int location) {

	for (int i = 0; i < HAMMING1_SIZE; i++) {
		if ((1 << i) & location) {
			//update the parity bit after changing
			(*parityBits) ^= 1 << i;
		}
	}
	
}
bool tryChangeBit(char* block, unsigned int tryToChange, unsigned int location) {
	//so that there is no exception
	if (location > 2) {
		Change_bit_for_all_Parity_bits(&tryToChange, location);
		//if is the adjacent bit that was replaced with the resulting location
		if (tryToChange == 0) {
			//update the original data
			change_bit_in_data(block, location);
			return true;
		}
	}
	return false;
}
void twoBitSwapped(char* block, unsigned int differnceHaming, unsigned int differnceHamingOdd) {
	char tryToChange = differnceHaming;
	//calculation to get the adjacent bit
	char addOne = (differnceHamingOdd + 1) & (differnceHamingOdd) ? (differnceHamingOdd + 1) : (log2(differnceHamingOdd + 1) + differnceHamingOdd);
	char subOne = (differnceHamingOdd - 1) & (differnceHamingOdd - 2) ? (differnceHamingOdd - 1) : (differnceHamingOdd - log2(differnceHamingOdd - 1));

	if (!tryChangeBit(block, tryToChange, addOne)) {
		tryChangeBit(block, tryToChange, subOne);
	}
}
void blockDecoder(char *block, ProtectionData* protection) {
	
	uint64_t parityBit = hamming_encode(block, BLOCK_SIZE, 1);
	unsigned int difference1 = compare_between_original_and_existing_haming_1(parityBit, (*protection).hamming1);
	print_binary(difference1);
	if (difference1 == 0) {
		return;
	}
	if (((*protection).parityBit) != MultyXor(block, BLOCK_SIZE)) {//האם הסיבית הראשונה שמכילה אקסור על כולם זהה לאקסור של כולם בעצם האם היתה שגיאה בודדת
	    change_bit_in_data(block, difference1);
	}
	else {
	
		parityBit = hamming_encode(block, BLOCK_SIZE, 2);
		int difference2 = compare_between_original_and_existing_haming_2(parityBit, (*protection).hamming2);
		//לשלוח לפונקציה של אסתר ואת התוצאה לשלוח לפונקציה שלמטה
		//ומכן ואילך לשלוח את מה שחזר בתור diferenve2
		//calculate_the_position_of_Hamming2_to_Hamming1
		change_bit_in_data(block, difference2);
		print_binary(difference1);
		Change_bit_for_all_Parity_bits(&difference1, difference2);
		twoBitSwapped(block, difference1, difference2);
	}
	
}

void decoder(void* block, int len, char* protection) {
	FILE* file = NULL;
	int index = 0;
	fopen_s(&file, protection, "rb"); // Read mode
	if (file == NULL) {
		perror("Error opening file");
		return;
	}

    fseek(file, 0, SEEK_END); // Move to end of file to get size
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET); // Move back to start of file
	while (index + BLOCK_SIZE <= len&& ftell(file) < file_size)
	{
		ProtectionData protectionData;
		fread(&protectionData, sizeof(ProtectionData), 1, file);
		blockDecoder(((char*)block + index), &protectionData);
		index += BLOCK_SIZE;
	}

	fclose(file);
}
int main()
{
	//int data = 3;
	//encode(&data, sizeof(data) * 8);
	int byte =2;
	ProtectionData protectionData;
	protectionData.parityBit = MultyXor(&byte, 8);
	protectionData.hamming1=hamming_encode(&byte, 8, 1);
	protectionData.hamming2=hamming_encode(&byte, 8, 2);
	byte = 1;
	//print_binary(byte);
	blockDecoder(&byte, &protectionData);
	//print_binary(byte);
	
}
//int main() {
//	int data = 3;
//	encode(&data, sizeof(data) * 8);
//	//רק לוודא שהכל פועל כשורה
//read_protection_data();
//}