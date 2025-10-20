#include "unity.h"

void tearDown(void) {
}

extern void test_decode_goodcrc(void);
extern void test_encode_goodcrc(void);
extern void test_pd_build_request_header(void);
extern void test_pd_build_request_header_no_data(void);

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_decode_goodcrc);
    RUN_TEST(test_encode_goodcrc);
    RUN_TEST(test_pd_build_request_header);
    RUN_TEST(test_pd_build_request_header_no_data);
    return UNITY_END();
}
