#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "time.h"
//#ifdef __INTEL_COMPILER
//#include <CL/sycl.hpp>
//#endif
extern "C" {
#include "encoder.h"
#include "decoder.h"
#include "files.h"
}
int compare_memory(const char* ptr1, const char* ptr2, size_t num) {
	for (size_t i = 0; i < num; i++) {
		if (ptr1[i] != ptr2[i]) {
			return 0;
		}
	}
	return 1;
}
int random_in_range(int x, int y) {
	srand(time(NULL));
	return rand() % (y - x + 1) + x;
}
//TEST_CASE("valid data") {
//	long data_length = 0;
//	std::cout << "enter a file or directory" << std::endl;
//	char file[30];
//	std::cin >> file;
//	char* data = read_file_to_string(file, &data_length);
//	encode(data, data_length * 8);
//	char* decoded = decode(pd, data_length * 8);
//	CHECK(compare_memory(data, decoded, data_length) == 1);
//}
////2
//TEST_CASE("data with bit flip") {
//	int place = random_in_range(1, HAMMING1_SIZE - 1);
//	long data_length = 0;
//	std::cout << "enter a file or directory" << std::endl;
//	char file[30];
//	std::cin >> file;
//	char* data = read_file_to_string(file, &data_length);
//	encode(data, data_length * 8);
//	change_bit_in_data(pd[0].hamming1, place);
//	char* decoded = decode(pd, sizeof(data) * 8);
//	CHECK(compare_memory(data, decoded, data_length) == 1);
//}
//3
//TEST_CASE("2 bits swapped in data") {
//	int place = random_in_range(1, HAMMING1_SIZE - 2);
//	int data[5] = { 1,2,3,4,5 };
//	encode(data, sizeof(data) * 8);
//	change_bit_in_data(pd[0].hamming1, place);
//	change_bit_in_data(pd[0].hamming1, place + 1);
//	char* decoded = decode(pd, sizeof(data) * 8);
//	CHECK(compare_memory((char*)data, decoded, sizeof(data)) == 1);
//}
//4
TEST_CASE("error in parity bit of data") {
	long data = 123456;
	encode(&data, sizeof(data) * 8);
	pd[0].parityBit = random_in_range(1, 2);
	char* decoded = decode(pd, sizeof(data) * 8);
	CHECK(compare_memory((char*)&data, decoded, sizeof(data)) == 1);
	long after = *(long*)decoded;
}
//5
//TEST_CASE("the block is smaller than the block size") {
//	char data[10] = { 'a','b','c','d','e','f','g','h','i','j' };
//	encode(&data, sizeof(data) * 8);
//	change_bit_in_data(pd[0].hamming1, 5);
//	char* decoded = decode(pd, sizeof(data) * 8);
//	CHECK(compare_memory((char*)data, decoded, sizeof(data)) == 1);
//}
//6
//TEST_CASE("data is null") {
//	char* data = NULL;
//	encode(data, sizeof(data) * 8);
//	char* decoded = decode(pd, sizeof(data) * 8);
//	CHECK(decoded == data);
//}
//7
TEST_CASE("bit from parity bit was swapped with bit from hamming1") {
	int data = 1000;
	encode(&data, sizeof(data) * 8);
	pd[0].parityBit = pd[0].parityBit == 0 ? 2 : 1;
	change_bit_in_data(pd[0].hamming1, HAMMING1_SIZE);
	char* decoded = decode(pd, sizeof(data) * 8);
	CHECK(compare_memory((char*)&data, decoded, sizeof(data)) == 1);
}
//8
TEST_CASE("bit flip in last place of byte") {
	int place = random_in_range(1, HAMMING1_SIZE / 8);
	short data = 67;
	encode(&data, sizeof(data) * 8);
	change_bit_in_data(pd[0].hamming1, place * 8);
	char* decoded = decode(pd, sizeof(data) * 8);
	CHECK(compare_memory((char*)&data, decoded, sizeof(data)) == 1);
}