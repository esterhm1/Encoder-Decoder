#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
extern "C" {
#include "encoder.h"
#include "decoder.h"
}

TEST_CASE("testing simple addition"){
    CHECK(1 + 1 == 2);
}

TEST_CASE("testing encode function") {
    int data = 7;
    int length = sizeof(data) * 8;
    ProtectionData* pd = encode(&data, length);
	ProtectionData* pd2 = (ProtectionData*)malloc(length / BLOCK_SIZE * sizeof(ProtectionData));
    //for (size_t i = 0; i < length / BLOCK_SIZE; i++) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            CHECK(pd[0][i] == pd2[0][i]);
        }
    //}
}