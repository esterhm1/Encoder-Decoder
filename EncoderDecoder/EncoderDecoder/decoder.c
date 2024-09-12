#include "decoder.h"
int hamming1_result(char* encoded_data)
{
	int result = 0;
	int num_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	for (size_t i = 1; i <= num_bits; i++)
	{
		int this_parity_bit = 0;
		for (size_t j = (size_t)pow(2, i - 1) + 1; j <= BLOCK_SIZE + num_bits; j++) {
			if (j & (size_t)pow(2, i - 1))
			{
				this_parity_bit ^= get_bit_value(encoded_data, j);
			}
		}
		if (this_parity_bit != get_bit_value(encoded_data, (size_t)pow(2, i - 1)))
		{
			result |= 1 << (i - 1);
		}
	}
	return result;
}
int hamming2_result(char* data, MyType hamming)
{
	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block((BLOCK_SIZE + number_of_parity_bits_for_block) / 2);
	int result = 0;
	for (size_t i = 1; i <= number_of_parity_bits; i++)
	{
		int this_parity_bit = 0;
		for (int j = (int)pow(2, i - 1) + 1; j <= (BLOCK_SIZE + number_of_parity_bits_for_block) / 2 + number_of_parity_bits; j++) {
			int index = get_adjusted_index(j, 2);
			if (index != -1 && (j & (int)pow(2, i - 1))) {
				this_parity_bit ^= get_bit_value(data, index);
			}
		}
		if (this_parity_bit != get_bit_value(&hamming, i))
		{
			result |= 1 << (i - 1);
		}
	}
	return get_adjusted_index(result, 2);
}
char* original_data(char* encoded_data)
{
	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;
	size_t num_bytes = (num_bits + 7) / 8; // Round up to nearest byte
	char* data = (char*)calloc(num_bytes, sizeof(char));
	if (data == NULL) {
		// Handle memory allocation failure
		return NULL;
	}
	for (size_t i = 1; i <= num_bits; i++)
	{
		if (!IS_POWER_OF_TWO(i))
		{
			if (get_bit_value(encoded_data, i))
			{
				int index = get_adjusted_index(i, 1);
				data[(index - 1) / 8] |= 1 << (8 - (index) % 8);
				if (index % 8 == 0)
					data[(index - 1) / 8] |= 1;
			}
		}
	}
	return data;
}
void change_bit_in_data(char* data, int place)
{
	if (place >= 0 && place <= HAMMING1_SIZE)
		data[(place - 1) / 8] ^= (1 << (8 - place % 8));
	if (place % 8 == 0)
	{
		data[(place - 1) / 8] ^= 1;
	}
}
int try_change_bit(char* data, int place)
{
	change_bit_in_data(data, place);
	return  hamming1_result(data);
}
void two_bits_swapped(char* encoded_data, MyType hamming2)
{
	int error = hamming2_result(encoded_data, hamming2);
	try_change_bit(encoded_data, error);
	if (try_change_bit(encoded_data, error - 1) == 0)
		return;
	try_change_bit(encoded_data, error - 1);
	try_change_bit(encoded_data, error + 1);
}
char* block_decoder(ProtectionData pd)
{
	if (((pd.parityBit & 1) ^ (pd.parityBit >> 1) & 1))
	{
		printf("error in parity bit\n");
	}
	else
	{
		unsigned int error_place = hamming1_result(pd.hamming1);
		if (error_place == 0)
		{
			printf("valid\n");
		}
		else
		{
			if (pd.parityBit != parity_bit_of_data(pd.hamming1, HAMMING1_SIZE))
			{
				printf("bit flip in %d\n", error_place);
				change_bit_in_data(pd.hamming1, error_place);
			}
			else
			{
				printf("two bits swapped\n");
				two_bits_swapped(pd.hamming1, pd.hamming2);
			}
		}
	}
	return original_data(pd.hamming1);
}
ProtectionData* read_protection_data_from_file(const char* file_name, long* number_of_blocks)
{
	FILE* file = NULL;
	errno_t err;
	// פתיחת קובץ בינארי במצב כתיבה
	err = fopen_s(&file, "output.bin", "rb");
	// בדיקת אם הקובץ נפתח בהצלחה
	if (err != 0) {
		perror("Error opening file");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file); // גודל הקובץ בבתים
	rewind(file); // חזרה לתחילת הקובץ
	*number_of_blocks = file_size / sizeof(ProtectionData);
	ProtectionData* protection = (ProtectionData*)malloc(sizeof(ProtectionData) * (*number_of_blocks));
	for (size_t i = 0; i < *number_of_blocks; i++)
	{
		fread(&protection[i], sizeof(ProtectionData), 1, file);
	}
	fclose(file);
	return protection;
}
typedef struct {
	ProtectionData* protection;
	BlockData* decoded_data;
	size_t start;
	size_t end;
} ThreadDecodeParams;
DWORD WINAPI thread_decode_block(LPVOID param) {
	ThreadDecodeParams* params = (ThreadDecodeParams*)param;
	for (size_t i = params->start; i < params->end; i++) {
		char* block = block_decoder(params->protection[i]);
		int num_bytes = BLOCK_SIZE / 8;
		memcpy(params->decoded_data[i].data, block, num_bytes);
	}
	return 0;
}
char* decode(ProtectionData* protection, int number_of_blocks) {
	BlockData* decoded_data = (BlockData*)malloc(number_of_blocks * sizeof(BlockData));
	// התאמת מספר הטרדים לגודל הדאטה
	size_t optimal_thread_count = (number_of_blocks < 8) ? number_of_blocks : 8;
	HANDLE* threads = (HANDLE*)malloc(optimal_thread_count * sizeof(HANDLE));
	ThreadDecodeParams* thread_params = (ThreadDecodeParams*)malloc(optimal_thread_count * sizeof(ThreadDecodeParams));
	size_t blocks_per_thread = number_of_blocks / optimal_thread_count;
	size_t remaining_blocks = number_of_blocks % optimal_thread_count;
	// יצירת הטרדים בהתאם לגודל הדאטה
	for (size_t i = 0; i < optimal_thread_count; i++) {
		thread_params[i].protection = protection;
		thread_params[i].decoded_data = decoded_data;
		thread_params[i].start = i * blocks_per_thread;
		thread_params[i].end = (i + 1) * blocks_per_thread;
		if (i == optimal_thread_count - 1) {
			thread_params[i].end += remaining_blocks;  // בלוקים שנותרו ייכנסו לטרד האחרון
		}
		threads[i] = CreateThread(NULL, 0, thread_decode_block, &thread_params[i], 0, NULL);
		if (threads[i] == NULL) {
			perror("Error creating thread");
			free(decoded_data);
			free(protection);
			free(threads);
			free(thread_params);
			return NULL;
		}
	}
	WaitForMultipleObjects(optimal_thread_count, threads, TRUE, INFINITE);
	// סגירת הטרדים
	for (size_t i = 0; i < optimal_thread_count; i++) {
		CloseHandle(threads[i]);
	}
	// שחרור הזיכרון
	free(threads);
	free(thread_params);
	return (char*)decoded_data;
}
void decode_files(const char* file_name)
{
	long number_of_blocks = 0;
	ProtectionData* protection = read_protection_data_from_file(file_name, &number_of_blocks);
	if (protection == NULL)
		return NULL;
	char* data = decode(protection, number_of_blocks);
	write_string_to_file("output.txt", data, number_of_blocks * BLOCK_SIZE / 8);
}
//char* decode(const char* file_name,int len)
//{
//	int number_of_blocks = len / BLOCK_SIZE;
//	if (len % BLOCK_SIZE)
//		number_of_blocks++;
//
//	ProtectionData* protection = read_protection_data_from_file(file_name, number_of_blocks);
//	if (protection == NULL)
//		return NULL;
//	BlockData* decoded_data=(BlockData*)malloc(number_of_blocks*sizeof(BlockData));
//
//	for (size_t i = 0; i < number_of_blocks; i++)
//	{
//		char* block = block_decoder(protection[i]);
//		int num_bytes = BLOCK_SIZE / 8;
//		memcpy(decoded_data[i].data, block, num_bytes);
//
//	}
//	return decoded_data;
//
//}



//#include "decoder.h"
//int hamming1_result(char* encoded_data)
//{
//	int result = 0;
//	int num_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
//	for (size_t i = 1; i <= num_bits; i++)
//	{
//		int this_parity_bit = 0;
//		for (size_t j = (size_t)pow(2, i - 1) + 1; j <= BLOCK_SIZE + num_bits; j++) {
//			if (j & (size_t)pow(2, i - 1))
//			{
//				this_parity_bit ^= get_bit_value(encoded_data, j);
//			}
//}
//
//void correct_one_bit_flip(char* dataWithHamming, int place) {
//	(*dataWithHamming) ^= ((int)pow(2, (BLOCK_SIZE_WITH_HAMMING - place)));
//}
//		if (this_parity_bit != get_bit_value(encoded_data, (size_t)pow(2, i - 1)))
//		{
//			result |= 1 << (i - 1);
//}
//void Change_bit_for_all_Parity_bits(unsigned int* parityBits, int location) {
//
//	for (int i = 0; i < HAMMING1_SIZE; i++) {
//		if ((1 << i) & location) {
//			//update the parity bit after changing
//			(*parityBits) ^= 1 << i;
//		}
//	return result;
//	}
//int hamming2_result(char* data, MyType hamming)
//{
//	int number_of_parity_bits_for_block = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
//	int number_of_parity_bits = calculate_number_of_parity_bits_for_block((BLOCK_SIZE + number_of_parity_bits_for_block) / 2);
//	int result = 0;
//	for (size_t i = 1; i <= number_of_parity_bits; i++)
//	{
//		int this_parity_bit = 0;
//		for (int j = (int)pow(2, i - 1) + 1; j <= (BLOCK_SIZE + number_of_parity_bits_for_block) / 2 + number_of_parity_bits; j++) {
//			int index = get_adjusted_index(j, 2);
//			if (index != -1 && (j & (int)pow(2, i - 1))) {
//				this_parity_bit ^= get_bit_value(data, index);
//			}
//}
//unsigned int compare_between_original_and_existing_haming_1(char* dataWithHamming, char* existingHamming) {
//	unsigned int difference = (!(getBit(dataWithHamming, 0) == getBit(existingHamming, 0)) << (HAMMING1_SIZE - 1));
//	for (int i = 1; i < BLOCK_SIZE_WITH_HAMMING; i *= 2) {
//		//update the parity bit the dataWithHamming difference
//		difference ^= (!(getBit(dataWithHamming, i) == getBit(existingHamming, i)) << (HAMMING1_SIZE - i - 1));
//	}
//	if (this_parity_bit != get_bit_value(&hamming, i))
//	{
//		result |= 1 << (i - 1);
//	}
//}
//unsigned int compare_between_original_and_existing_haming_2(char* originalHamming, char* existingHamming) {
//	unsigned int difference = 0;
//	for (int i = 0; i < HAMMING2_SIZE; i++) {
//		//update the parity bit the specific difference
//		difference ^= (!(getBit(originalHamming,i) == getBit(existingHamming, i)) << (HAMMING2_SIZE - i - 1));
//	}
//	return get_adjusted_index(result, 2);
//}
//char* original_data(char* encoded_data)
//{
//	int number_of_parity_bits = calculate_number_of_parity_bits_for_block(BLOCK_SIZE);
//	size_t num_bits = BLOCK_SIZE + number_of_parity_bits;
//	size_t num_bytes = (num_bits + 7) / 8; // Round up to nearest byte
//	char* data = (char*)calloc(num_bytes, sizeof(char));
//	if (data == NULL) {
//		// Handle memory allocation failure
//		return NULL;
//		}
//	for (size_t i = 1; i <= num_bits; i++)
//	{
//		if (!IS_POWER_OF_TWO(i))
//		{
//			if (get_bit_value(encoded_data, i))
//			{
//				int index = get_adjusted_index(i, 1);
//				data[(index - 1) / 8] |= 1 << (8 - (index) % 8);
//				if (index % 8 == 0)
//					data[(index - 1) / 8] |= 1;
//	}
//		}
//	}
//	return data;
//}
//void change_bit_in_data(char* data, int place)
//{
//	data[(place - 1) / 8] ^= (1 << ((8 - place) % 8));
//}
//int try_change_bit(char* data, int place)
//{
//	change_bit_in_data(data, place);
//	return  hamming1_result(data);
//}
//	if (!tryChangeBit(dataWithHamming, differnceHamingWithData, differnceHamingOdd + 1)) {
//		tryChangeBit(dataWithHamming, differnceHamingWithData, differnceHamingOdd - 1);
//	}
//void two_bits_swapped(char* encoded_data, MyType hamming2)
//{
//	int error = hamming2_result(encoded_data, hamming2);
//	try_change_bit(encoded_data, error);
//	if (try_change_bit(encoded_data, error - 1) == 0)
//		return;
//	try_change_bit(encoded_data, error - 1);
//	try_change_bit(encoded_data, error + 1);
//}
//char* block_decoder(ProtectionData pd)
//{
//	if (((pd.parityBit & 1) ^ (pd.parityBit >> 1) & 1))
//		return original_data(pd.hamming1);
//	else
//	{
//		unsigned int error_place = hamming1_result(pd.hamming1);
//		if (error_place == 0)
//		{
//			printf("valid\n");
//			return original_data(pd.hamming1);
//}
//		else
//		{
//			if (pd.parityBit != parity_bit_of_data(pd.hamming1, HAMMING1_SIZE))
//			{
//
//				printf("bit flip in %d\n", error_place);
//				change_bit_in_data(pd.hamming1, error_place);
//				return original_data(pd.hamming1);
//}
//			else
//			{
//				printf("two bits swapped\n");
//				two_bits_swapped(pd.hamming1, pd.hamming2);
//	}
//	char* data = extract_data(protection_struct->dataWithHamming);
//	// the function hamming_decoder only find the place that may be an error
//	unsigned int error_place = hamming_decoder(data,protection_struct->dataWithHamming);
//
//	if (error_place == 0) { // There is no error
//		return data; // הפונקציה הזו מחלצת את המידע  
//	}
//
//	// now, there is an error and the two parityBit are the same!!!
//	// if the XOR on dataWithHamming is not the same as the first/ second (same) in other words it's equal to 1 - there is one bit flip
//	if (MultyXor((*protection_struct).dataWithHamming) != (*protection_struct).parityBit & 1) {
//		correct_one_bit_flip((*protection_struct).dataWithHamming, error_place);
//	}
//	return original_data(pd.hamming1);
//	}
//char* decode(ProtectionData* pd, int len)
//{
//	int number_of_blocks = len / BLOCK_SIZE;
//	if (len % BLOCK_SIZE)
//		number_of_blocks++;
//	char* decoded_data = (char*)malloc(len);
//	char* data_ptr = decoded_data;
//	if (pd == NULL) {
//		fprintf(stderr, "Error: Null pointer encountered.\n");
//		return decoded_data;
//}
//	for (size_t i = 0; i < number_of_blocks; i++)
//	{
//		char* block = block_decoder(pd[i]);
//		int num_bytes = BLOCK_SIZE / 8;
//		memcpy(data_ptr, block, num_bytes);
//		data_ptr += num_bytes;
//	}
//	return decoded_data;
//}