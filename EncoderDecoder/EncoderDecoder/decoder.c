//#include <stdbool.h>
//#include "encoder.c"
//unsigned int compare_between_original_and_existing_haming_1(uint64_t originalHamming, unsigned int existingHamming) {
//	unsigned int difference = 0;
//	for (int i = 0; i < HAMMING1_SIZE; i++) {
//		//update the parity bit the specific difference
//		difference ^= (!(!!(originalHamming >> i & 1) == !!(existingHamming >> i & 1)) << (HAMMING1_SIZE - i - 1));
//	}
//	return difference;
//}
//unsigned int compare_between_original_and_existing_haming_2(uint64_t originalHamming, unsigned int existingHamming) {
//	unsigned int difference = 0;
//	for (int i = 0; i < HAMMING2_SIZE; i++) {
//		//update the parity bit the specific difference
//		difference ^= (!(!!(originalHamming >> i & 1) == !!(existingHamming >> i & 1)) << (HAMMING2_SIZE - i - 1));
//	}
//	return difference;
//}
//void change_bit_in_data(char* block, unsigned int location) {
//	(*block) ^= ((int)pow(2, BLOCK_SIZE - (location - (int)log2(location) - 1)));
//}
//
//void Change_bit_for_all_Parity_bits(unsigned int* parityBits, int location) {
//
//	for (int i = 0; i < HAMMING1_SIZE; i++) {
//		if ((1 << i) & location) {
//			//update the parity bit after changing
//			(*parityBits) ^= 1 << i;
//		}
//	}
//
//}
//bool tryChangeBit(char* block, unsigned int tryToChange, unsigned int location) {
//	//so that there is no exception
//	if (location > 2) {
//		Change_bit_for_all_Parity_bits(&tryToChange, location);
//		//if is the adjacent bit that was replaced with the resulting location
//		if (tryToChange == 0) {
//			//update the original data
//			change_bit_in_data(block, location);
//			return true;
//		}
//	}
//	return false;
//}
//void twoBitSwapped(char* block, unsigned int differnceHaming, unsigned int differnceHamingOdd) {
//	char tryToChange = differnceHaming;
//	//calculation to get the adjacent bit
//	char addOne = (differnceHamingOdd + 1) & (differnceHamingOdd) ? (differnceHamingOdd + 1) : (log2(differnceHamingOdd + 1) + differnceHamingOdd);
//	char subOne = (differnceHamingOdd - 1) & (differnceHamingOdd - 2) ? (differnceHamingOdd - 1) : (differnceHamingOdd - log2(differnceHamingOdd - 1));
//
//	if (!tryChangeBit(block, tryToChange, addOne)) {
//		tryChangeBit(block, tryToChange, subOne);
//	}
//}
//void blockDecoder(char* block, ProtectionData* protection) {
//
//	uint64_t parityBit = hamming_encode(block, BLOCK_SIZE, 1);
//	unsigned int difference1 = compare_between_original_and_existing_haming_1(parityBit, (*protection).hamming1);
//	print_binary(difference1);
//	if (difference1 == 0) {
//		return;
//	}
//	if (((*protection).parityBit) != MultyXor(block, BLOCK_SIZE)) {//האם הסיבית הראשונה שמכילה אקסור על כולם זהה לאקסור של כולם בעצם האם היתה שגיאה בודדת
//		change_bit_in_data(block, difference1);
//	}
//	else {
//
//		parityBit = hamming_encode(block, BLOCK_SIZE, 2);
//		int difference2 = compare_between_original_and_existing_haming_2(parityBit, (*protection).hamming2);
//		//לשלוח לפונקציה של אסתר ואת התוצאה לשלוח לפונקציה שלמטה
//		//ומכן ואילך לשלוח את מה שחזר בתור diferenve2
//		//calculate_the_position_of_Hamming2_to_Hamming1
//		change_bit_in_data(block, difference2);
//		print_binary(difference1);
//		Change_bit_for_all_Parity_bits(&difference1, difference2);
//		twoBitSwapped(block, difference1, difference2);
//	}
//
//}
//
//void decoder(void* block, int len, char* protection) {
//	FILE* file = NULL;
//	int index = 0;
//	fopen_s(&file, protection, "rb"); // Read mode
//	if (file == NULL) {
//		perror("Error opening file");
//		return;
//	}
//
//	fseek(file, 0, SEEK_END); // Move to end of file to get size
//	long file_size = ftell(file);
//	fseek(file, 0, SEEK_SET); // Move back to start of file
//	while (index + BLOCK_SIZE <= len && ftell(file) < file_size)
//	{
//		ProtectionData protectionData;
//		fread(&protectionData, sizeof(ProtectionData), 1, file);
//		blockDecoder(((char*)block + index), &protectionData);
//		index += BLOCK_SIZE;
//	}
//
//	fclose(file);
//}
//
