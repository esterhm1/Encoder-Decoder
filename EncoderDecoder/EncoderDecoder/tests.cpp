#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
extern "C" {
#include "encoder.h"
#include "decoder.h"
}

// To change the data of the tests!!!!
TEST_CASE("testing simple addition") {
    int data = 1;
    encode(&data, sizeof(data) * 8);
    change_bit_in_data(pd[0].hamming1, 1);
    change_bit_in_data(pd[0].hamming1, 2);
    char* decoded = decode(pd, sizeof(data) * 8);
    CHECK(data == *decoded);
}

TEST_CASE("Bit flip in the data") {
    int data = 1;
    encode(&data, sizeof(data) * 8);
    change_bit_in_data(pd[0].hamming1, 10);
    char* decoded = decode(pd, sizeof(data) * 8);
    CHECK(data == *decoded);
}

TEST_CASE("Bit flip in the parity bit of hamming 1") {
    int data = 1;
    encode(&data, sizeof(data) * 8);
    change_bit_in_data(pd[0].hamming1, 3);
    char* decoded = decode(pd, sizeof(data) * 8);
    CHECK(data == *decoded);
}
 
TEST_CASE("Bit flip in the parity bit - multy XOR") {
    int data = 1;
    encode(&data, sizeof(data) * 8);
    toggle_parity_bit_of_all_the_data(0);
    char* decoded = decode(pd, sizeof(data) * 8);
    CHECK(data == *decoded);
}

TEST_CASE("Bit flip in hamming 2") {
    int data = 1;
    encode(&data, sizeof(data) * 8);
    toggle_hamming2_bit(&pd->hamming2, 3);
    char* decoded = decode(pd, sizeof(data) * 8);
    CHECK(data == *decoded);
}

TEST_CASE("More than two bits flip in hamming 2") {
    int data = 1;
    encode(&data, sizeof(data) * 8);
    toggle_hamming2_bit(&pd->hamming2, 3);
    toggle_hamming2_bit(&pd->hamming2, 4);
    toggle_hamming2_bit(&pd->hamming2, 2);
    toggle_hamming2_bit(&pd->hamming2, 0);
    char* decoded = decode(pd, sizeof(data) * 8);
    CHECK(data == *decoded);
}

