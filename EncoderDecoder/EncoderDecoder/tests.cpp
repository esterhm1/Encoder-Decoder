#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
extern "C" {
#include "encoder.h"
#include "decoder.h"
}

TEST_CASE("testing simple addition"){
    CHECK(1 + 1 == 2);
}

