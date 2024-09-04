#ifdef BUILD_MAIN
#include "decoder.h"

int main()
{
	/*int data = 57845;
	ProtectionData* pd=encode(&data, sizeof(int) * 8);
	for (size_t i = 0; i < 4; i++)
	{
		printf("%d\n", pd[i].parityBit);
		print_binary(pd[i].hamming1, 2);
	}*/

	/*int data = 32;
	char* encoded = hamming_with_data(&data, 1);
	char* decoded = original_data(encoded);
	binary_represent(decoded,8);*/


	int data = 7;
	int len = sizeof(data) * 8;
	printf("%d\n", len);
	ProtectionData* encoded = encode(&data, len);
	printf("\n");
	binary_represent(&encoded[0].hamming1[0], BLOCK_SIZE + calculate_number_of_parity_bits_for_block(BLOCK_SIZE));
	printf("\n");
	encoded[0].hamming1[0] = 161;
	binary_represent(&encoded[0].hamming1[0], BLOCK_SIZE + calculate_number_of_parity_bits_for_block(BLOCK_SIZE));
	printf("\n");
	char* decoded = decode(encoded, len);
	binary_represent(&data, len);
	binary_represent(decoded, len);
}
#endif