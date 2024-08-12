#include <stdint.h>
#include<stdio.h>
#include <math.h>
#include <stdlib.h>
#include "Encoder.h"
#define _CRT_SECURE_NO_WARNINGS

int MultyXor(void* data) {
	unsigned char* bytePtr = (unsigned char*)data;
	int parity = 0;

	for (size_t i = 0; i < BLOCK_SIZE; ++i) {
		unsigned char byte = bytePtr[i];

		for (int bit = 0; bit < 8; ++bit) {

			uint8_t bitValue = (byte >> bit) & 1;

			parity ^= bitValue;
		}
	}
	return parity;
}

int calculateNumberOfParityBitsForBlock(int block_length){
	int counter = 0;
	while ((1 << counter) < (block_length + counter + 1)) {
		counter++;
	}

	return counter;
}

int isPowerOfTwo(size_t num) {
	return (num & (num - 1)) == 0;
}

int getAdjustedIndex(int index, int skipped) {
	// If the index is a power of 2, return -1.
	if (isPowerOfTwo(index)) {
		return -1;
	}

	// Otherwise, calculate the adjusted index.
	int adjustedIndex = 1;
	for (size_t i = 1; i < index; ++i) {
		if (!isPowerOfTwo(i)) {
			++adjustedIndex;
		}
	}
	if (skipped == 1)
		return adjustedIndex;
	return adjustedIndex * 2 - 1;
}

int getBitValue(const void* data, int bitPosition) {
	bitPosition--;
	// Pointer conversion to unsigned char*to access the data as bytes
	const unsigned char* bytePtr = (const unsigned char*) data;

	// Calculate the position of the byte and the bit within the byte.
	int byteIndex = (bitPosition) / 8;
	int bitIndex = 7 - ((bitPosition) % 8); // 7 - bitIndex כדי לקרוא ביטים משמאל לימין

	// Verify the location of the byte is within the boundaries of the memory area
	// (Suppose the memory area is smaller than an array of unsigned char)
	if (byteIndex < 0 || byteIndex >= sizeof(data)) {
		return -1; // Error: Position out of range
	}

	// Removing the appropriate byte from the pointer
	unsigned char byte = bytePtr[byteIndex];

	// Create a mask to check the corresponding bit value
	unsigned char mask = 1 << (bitIndex);

	// Using a mask to check the corresponding bit value
	int bitValue = (byte & mask) >> bitIndex;

	return bitValue;
}

void binaryRepresent(void* data, int bitsNumber) {
	// Pointer to the data as unsigned char
	unsigned char* bytePtr = (unsigned char*) data;

	// Calculation of the total number of bytes
	int totalBytes = (bitsNumber + 7) / 8; // Round up to include all bytes

	// Prints the bits
	for (int i = 0; i < totalBytes; ++i) {
		unsigned char byte = bytePtr[i];

		// Ensure that no more bits are printed than the given data.
		int bitsToPrint = (i == totalBytes - 1) ? (bitsNumber % 8) : 8;
		if (bitsToPrint == 0) bitsToPrint = 8; // If it's the first byte 

		for (int bit = bitsToPrint - 1; bit >= 0; --bit) {
			printf("%d ", (byte >> bit) & 1);
		}
	}

	printf("\n");
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
	}

	printf("\n");
}


int hammingEncode(const void* data, int skipped) {
	int numberOfParityBits = calculateNumberOfParityBitsForBlock(BLOCK_SIZE / skipped);
	int result = 0;

	// Compute and store the parity bits
	for (int i = 1; i <= numberOfParityBits; i++) {
		int thisParityBit = 0;

		for (int j = (int)pow(2, i - 1); j <= BLOCK_SIZE / skipped + numberOfParityBits; j++) {
			int index = getAdjustedIndex(j, skipped);

			if (index != -1 && (j & (int)pow(2, i - 1))) {
				thisParityBit ^= getBitValue(data, index);
			}
		}

		// Store the parity bit in the result
		if (thisParityBit) {
			result |= (1ULL << (numberOfParityBits - i));
		}
	}

	return result;
}


void blockEncode(void* block, char* protectionFileName){
	ProtectionData protection;
	protection.parityBit = MultyXor(block);
	protection.hamming1 = hammingEncode(block, 1);
	protection.hamming2 = hammingEncode(block, 2);
	FILE* file = NULL;
	fopen_s(&file, protectionFileName, "ab");
	fwrite(&protection, sizeof(protection), 1, file);
	fclose(file);
}

void encode(char* data, long length){
	char* protectionFileName = "output.bin";
	void* ptr;
	int index = 0;
	while (index + BLOCK_SIZE <= length)
	{
		blockEncode(data, protectionFileName);
		index += BLOCK_SIZE;
		data += BLOCK_SIZE / 8;
	}
}

void readProtectionData() {
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
		printBinary(d.hamming1);
		printBinary(d.hamming2);

	}

	fclose(file);
}