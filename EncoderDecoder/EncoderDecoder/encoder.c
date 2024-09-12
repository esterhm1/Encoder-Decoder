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
int calculate_number_of_parity_bits_for_block(int num)
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
	// Pointer conversion to unsigned char*to access the data as bytes
	const unsigned char* bytePtr = (const unsigned char*) data;

	// Calculate the position of the byte and the bit within the byte.
	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8);
	if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1;
	}

	unsigned char byte = data[byteIndex];
	unsigned char mask = 1 << (bitIndex);

	// Using a mask to check the corresponding bit value
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

void printBinary(unsigned int num) {
	const int BITS = sizeof(num) * 8;  // The number of bits in unsigned int

	// Print the bit from the MSB to the LSB
	for (int i = BITS - 1; i >= 0; --i) {
		printf("%d", (num >> i) & 1);

		// Space after 8 bits
		if (i % 8 == 0 && i != 0) {
			printf(" ");
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
//void encode(char* data, long length) {
//	if (data == NULL)
//	{
//		pd = NULL;
//		return;
//	}
//
//	size_t full_blocks = length / BLOCK_SIZE;
//	size_t last_block_size = length % BLOCK_SIZE;
//	size_t final_blocks_num = full_blocks;
//	if (last_block_size)
//		final_blocks_num++;
//	pd = (ProtectionData*)malloc((final_blocks_num) * sizeof(ProtectionData));
//	if (pd == NULL) {
//		perror("Error allocating memory");
//		return;
//	}
//	for (size_t i = 0; i < full_blocks; i++) {
//		block_encode(data, i);
//		data += BLOCK_SIZE / 8;
//	}
//	if (last_block_size > 0)
//	{
//		char* last_block = (char*)malloc((BLOCK_SIZE / 8));
//		if (last_block == NULL) {
//			perror("Error allocating memory for last block");
//			free(pd);
//			return;
//		}
//		memset(last_block, 0, NUM_BYTES(BLOCK_SIZE));
//		memcpy(last_block, data, NUM_BYTES(last_block_size));
//		block_encode(last_block, full_blocks);
//		free(last_block);
//	}
//	//FILE* file = NULL;
//	//errno_t err;
//	//// פתיחת קובץ בינארי במצב כתיבה
//	//err = fopen_s(&file, "output.bin", "wb");
//	//// בדיקת אם הקובץ נפתח בהצלחה
//	//if (err != 0) {
//	//	perror("Error opening file");
//	//	return 1;
//	//}
//	//for (size_t i = 0; i < final_blocks_num; i++)
//	//{
//	//	size_t result = fwrite(&pd[i], sizeof(ProtectionData), 1, file);
//	//	if (result != 1) {
//	//		perror("Error writing to file");
//	//		fclose(file);
//	//		return 1;
//	//	}
//	//}
//
//	//fclose(file);
//	//free(pd);
//}

//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#define NUM_BYTES(x) ((x + 7) / 8) // פונקציה עוזרת לחישוב מספר הבתים הנדרשים
//#define NUM_THREADS 4
//
//typedef struct {
//	char* data;
//	size_t start;
//	size_t end;
//} ThreadData;
//
//// פונקציה שתוקדד על ידי ה-thread
//DWORD WINAPI ThreadFunction(LPVOID lpParam) {
//	ThreadData* threadData = (ThreadData*)lpParam;
//	char* data = threadData->data;
//	size_t start = threadData->start;
//	size_t end = threadData->end;
//
//	// קידוד בלוקים מלאים
//	for (size_t i = start; i < end; i++) {
//		block_encode(data , i);
//		data += BLOCK_SIZE / 8;
//	}
//
//	return 0;
//}
//
//void encode(char* data, long length) {
//
//	// חישוב מספר הבלוקים המלאים
//	size_t full_blocks = length / BLOCK_SIZE;
//	// חישוב הגודל של הבלוק האחרון אם הוא חלקי
//	size_t last_block_size = length % BLOCK_SIZE;
//	size_t final_blocks_num = full_blocks;
//	if (last_block_size)
//		final_blocks_num++;
//	pd = (ProtectionData*)malloc((final_blocks_num) * sizeof(ProtectionData));
//	if (pd == NULL) {
//		perror("Error allocating memory");
//		return;
//	}
//	
//	HANDLE threads[NUM_THREADS];
//	DWORD threadId;
//	ThreadData threadData[NUM_THREADS];
//
//	size_t block_size = full_blocks / NUM_THREADS;
//	size_t remainder = full_blocks % NUM_THREADS;
//	size_t start = 0;
//
//	for (int i = 0; i < NUM_THREADS; i++) {
//		threadData[i].data = data;
//		threadData[i].start = start;
//		threadData[i].end = (i == NUM_THREADS - 1) ? full_blocks : start + block_size;
//
//		threads[i] = CreateThread(
//			NULL,                // הגדרות ברירת מחדל
//			0,                   // גודל הערימה (0 = ברירת מחדל)
//			ThreadFunction,      // פונקציה שתוקדד
//			&threadData[i],      // פרמטר לפונקציה
//			0,                   // מצב הפעלת ה-thread (0 = מיד)
//			&threadId            // מזהה ה-thread
//		);
//
//		if (threads[i] == NULL) {
//			printf("Error creating thread %d: %lu\n", i, GetLastError());
//			return;
//		}
//
//		start = threadData[i].end;
//	}
//
//	// המתן ל-threads להסתיים
//	WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
//
//	// סגור את הידיות של ה-threads
//	for (int i = 0; i < NUM_THREADS; i++) {
//		CloseHandle(threads[i]);
//	}
//
//	// טיפול בבלוק האחרון אם הוא חלקי
//	if (last_block_size > 0) {
//		// יצירת בלוק חדש בגודל BLOCK_SIZE
//		char* last_block = (char*)malloc((BLOCK_SIZE / 8));
//		if (last_block == NULL) {
//			perror("Error allocating memory for last block");
//			return;
//		}
//		// אתחול יתרת הבלוק לאפסים
//		memset(last_block, 0, NUM_BYTES(BLOCK_SIZE));
//		// העתקת הבלוק האחרון מהנתונים
//		memcpy(last_block, data + full_blocks * BLOCK_SIZE / 8, NUM_BYTES(last_block_size));
//		// קידוד הבלוק האחרון
//		block_encode(last_block, full_blocks);
//		// ניקוי
//		free(last_block);
//	}
//}

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