//
//extern "C" {
//#include "decoder.h"
//#include "files.h"
//}
//
//
//int main()
//{
//	unsigned long long int data_length = 0;
//	char* data = read_file_to_string("input.jpg", &data_length);
//	encode(data, data_length * 8);
//	int len = data_length * 8;
//	int length = len / BLOCK_SIZE;
//	if (len % BLOCK_SIZE)
//		length++;
//	/*ProtectionData* protection = read_protection_data_from_file("output.bin", length);
//	for (size_t i = 0; i < length; i++)
//	{
//		change_bit_in_data(protection[i].hamming1, 6);
//		change_bit_in_data(protection[i].hamming1, 7);
//
//	}	
//	write_protection_data_to_file("output.bin", protection, length);*/
//	char* decoded = decode("output.bin", data_length * 8);
//	write_string_to_file("output.jpg", decoded, data_length);
//	
//}
//
//
//
//
//
//
//
