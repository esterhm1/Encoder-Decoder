#ifdef BUILD_MAIN
#include "decoder.h"
#include "files.h"

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


		char a[3]={9,7,7};
		int len = sizeof(a) * 8;
		encode(a, len);
		printf("before "); 
		binary_represent(pd, NUM_BYTES(HAMMING1_SIZE + HAMMING2_SIZE)*8);
		/*change_bit_in_data(pd[0].hamming1, 1);
		change_bit_in_data(pd[0].hamming1, 2);
		printf("after  ");
		binary_represent(pd[0].hamming1, NUM_BYTES(HAMMING1_SIZE)*8);
		char* decoded=decode(pd, sizeof(a) * 8);
		binary_represent(decoded, sizeof(a) * 8);*/


	//int data = 7;
	//int len = sizeof(data) * 8;
	//printf("%d\n", len);
	//ProtectionData* encoded = encode(&data, len);
	//printf("\n");
	//binary_represent(&encoded[0].hamming1[0], BLOCK_SIZE + calculate_number_of_parity_bits_for_block(BLOCK_SIZE));
	//printf("\n");
	//encoded[0].hamming1[0] = 161;
	//binary_represent(&encoded[0].hamming1[0], BLOCK_SIZE + calculate_number_of_parity_bits_for_block(BLOCK_SIZE));
	//printf("\n");
	//char* decoded = decode(encoded, len);
	//binary_represent(&data, len);
	//binary_represent(decoded, len);
	//-------------------------------------------------
	//const char* input_filename = "input.png";
	//const char* output_filename = "output.png";
	//long length;
	//char* content = read_file_to_string(input_filename, &length);
	//if (content == NULL) {
	//	return 1; // ëéùìåï á÷øéàä
	//}

	//encode(content, length * 8);
	//for (size_t i = 1; i < 20; i++)
	//{
	//	change_bit_in_data(pd[i].hamming1, 3);
	//	change_bit_in_data(pd[i].hamming1, 4);
	//}


	//char* decoded = decode(pd, length * 8);

	//// ëúéáä ìúåê ÷åáõ çãù
	//if (write_string_to_file(output_filename, decoded, length) != 0) {
	//	free(content);
	//	return 1; // ëéùìåï áëúéáä
	//}

	//free(content);
	return 0; // äöìçä 
}
#endif