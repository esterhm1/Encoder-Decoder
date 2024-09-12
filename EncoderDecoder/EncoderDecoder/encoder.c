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
//	//// ����� ���� ������ ���� �����
//	//err = fopen_s(&file, "output.bin", "wb");
//	//// ����� �� ����� ���� ������
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
//#define NUM_BYTES(x) ((x + 7) / 8) // ������� ����� ������ ���� ����� �������
//#define NUM_THREADS 4
//
//typedef struct {
//	char* data;
//	size_t start;
//	size_t end;
//} ThreadData;
//
//// ������� ������ �� ��� �-thread
//DWORD WINAPI ThreadFunction(LPVOID lpParam) {
//	ThreadData* threadData = (ThreadData*)lpParam;
//	char* data = threadData->data;
//	size_t start = threadData->start;
//	size_t end = threadData->end;
//
//	// ����� ������ �����
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
//	// ����� ���� ������� ������
//	size_t full_blocks = length / BLOCK_SIZE;
//	// ����� ����� �� ����� ������ �� ��� ����
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
//			NULL,                // ������ ����� ����
//			0,                   // ���� ������ (0 = ����� ����)
//			ThreadFunction,      // ������� ������
//			&threadData[i],      // ����� ��������
//			0,                   // ��� ����� �-thread (0 = ���)
//			&threadId            // ���� �-thread
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
//	// ���� �-threads �������
//	WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);
//
//	// ���� �� ������ �� �-threads
//	for (int i = 0; i < NUM_THREADS; i++) {
//		CloseHandle(threads[i]);
//	}
//
//	// ����� ����� ������ �� ��� ����
//	if (last_block_size > 0) {
//		// ����� ���� ��� ����� BLOCK_SIZE
//		char* last_block = (char*)malloc((BLOCK_SIZE / 8));
//		if (last_block == NULL) {
//			perror("Error allocating memory for last block");
//			return;
//		}
//		// ����� ���� ����� ������
//		memset(last_block, 0, NUM_BYTES(BLOCK_SIZE));
//		// ����� ����� ������ ��������
//		memcpy(last_block, data + full_blocks * BLOCK_SIZE / 8, NUM_BYTES(last_block_size));
//		// ����� ����� ������
//		block_encode(last_block, full_blocks);
//		// �����
//		free(last_block);
//	}
//}
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// ������� ���� �� ���
typedef struct {
	void* block;
	int index;
} ThreadParams;
DWORD WINAPI thread_encode_block(LPVOID param) {
	ThreadParams* params = (ThreadParams*)param;
	block_encode(params->block, params->index);
	free(params->block);  // ����� ������� �� ����� ���� ������
	free(params);  // ����� ������� �� �������� ���� ���� ������
	return 0;
}
void encode(char* data, long length) {
	if (data == NULL) {
		pd = NULL;
		return;
	}
	size_t full_blocks = length / BLOCK_SIZE;
	size_t last_block_size = length % BLOCK_SIZE;
	size_t final_blocks_num = full_blocks;
	if (last_block_size)
		final_blocks_num++;
	pd = (ProtectionData*)malloc(final_blocks_num * sizeof(ProtectionData));
	if (pd == NULL) {
		perror("Error allocating memory");
		return;
	}
	HANDLE* threads = (HANDLE*)malloc(full_blocks * sizeof(HANDLE));  // ���� ������
	if (threads == NULL) {
		perror("Error allocating memory for threads");
		free(pd);
		return;
	}
	// ����� ����� ��� ����
	for (size_t i = 0; i < full_blocks; i++) {
		// ���� ����� ������
		char* block = (char*)malloc(NUM_BYTES(BLOCK_SIZE));
		if (block == NULL) {
			perror("Error allocating memory for block");
			free(pd);
			free(threads);
			return;
		}
		memcpy(block, data, NUM_BYTES(BLOCK_SIZE));  // ����� ����� �����
		ThreadParams* params = (ThreadParams*)malloc(sizeof(ThreadParams));
		if (params == NULL) {
			perror("Error allocating memory for thread parameters");
			free(pd);
			free(threads);
			free(block);
			return;
		}
		params->block = block;
		params->index = i;
		// ����� ����
		threads[i] = CreateThread(NULL, 0, thread_encode_block, params, 0, NULL);
		if (threads[i] == NULL) {
			perror("Error creating thread");
			free(pd);
			free(threads);
			free(block);
			free(params);
			return;
		}
		data += BLOCK_SIZE / 8;  // ���� ����� ���
	}
	WaitForMultipleObjects(full_blocks, threads, TRUE, INFINITE);  // ����� ��� ������
	for (size_t i = 0; i < full_blocks; i++) {
		CloseHandle(threads[i]);  // ����� �� ���
	}
	// ����� ����� ������ �� �� ���� ����
	if (last_block_size > 0) {
		char* last_block = (char*)malloc(NUM_BYTES(BLOCK_SIZE));
		if (last_block == NULL) {
			perror("Error allocating memory for last block");
			free(pd);
			free(threads);
			return;
		}
		memset(last_block, 0, NUM_BYTES(BLOCK_SIZE));
		memcpy(last_block, data, NUM_BYTES(last_block_size));
		block_encode(last_block, full_blocks);
		free(last_block);
	}
	// ����� �����
	//write_protection_data_to_file("output.bin", pd, final_blocks_num);
	// ����� ������
	free(pd);
	free(threads);
}
void binary_represent(void* data, int bits_number) {
	unsigned char* byte_ptr = (unsigned char*)data;
	int total_bytes = (bits_number + 7) / 8;
	for (int i = 0; i < total_bytes; ++i) {
		unsigned char byte = byte_ptr[i];
		int bits_to_print = (i == total_bytes - 1) ? (bits_number % 8) : 8;
		if (bits_to_print == 0) bits_to_print = 8; // �� �� byte ������
		for (int bit = bits_to_print - 1; bit >= 0; --bit) {
			printf("%d ", (byte >> bit) & 1);
		}
	}
	printf("\n");
}
