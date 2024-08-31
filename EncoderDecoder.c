
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


	int data=300;
	binary_represent(&data, 16);
	int len = sizeof(data)*8;
	ProtectionData* encoded = encode(&data,len);
	encoded[0].hamming1[1] = 7;
	char* decoded = decode(encoded, len);
	binary_represent(decoded, 16);
	


}


