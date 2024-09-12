#pragma once
#define BLOCK_SIZE 8
#define HAMMING1_SIZE 4
#define HAMMING2_SIZE 3

#pragma pack(push, 1)
typedef struct ProtectionData {
	unsigned char parityBit : 2;
	unsigned short dataWithHamming : HAMMING1_SIZE;
	unsigned char oddHamming : HAMMING2_SIZE;
}ProtectionData;
#pragma pack(pop)

//#pragma pack(push, 1)
//typedef struct ProtectionData {
//	unsigned char parityBit : 1;
//	unsigned int hamming1 : HAMMING1_SIZE;
//	unsigned int hamming2 : HAMMING2_SIZE;
//}ProtectionData;
//#pragma pack(pop)

int MultyXor(void* data);
int calculateNumberOfParityBitsForBlock(int block_length);
int isPowerOfTwo(size_t num);
int getAdjustedIndex(int index, int skipped);
int getBitValue(const void* data, int bitPosition);
void binaryRepresent(void* data, int bitsNumber);
void printBinary(unsigned int num);
int hammingEncode(const void* data, int skipped);
void blockEncode(void* block, char* protectionFileName);
void encode(char* data, long length);
void readProtectionData();




