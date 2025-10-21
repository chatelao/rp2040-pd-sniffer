#include "unity.h"

void tearDown(void) {
}

extern void test_decode_goodcrc(void);
extern void test_encode_goodcrc(void);
extern void test_pd_build_request_header(void);
extern void test_pd_build_request_header_no_data(void);
extern void test_pinger_process_packet_pong(void);
extern void test_pinger_process_packet_not_pong(void);
extern void test_pinger_process_packet_invalid(void);
extern void test_ponger_process_packet_ping(void);
extern void test_ponger_process_packet_not_ping(void);
extern void test_ponger_process_packet_invalid(void);
extern void test_source_emulator_prepare_packet(void);
extern void test_sink_tester_process_packet_valid_source_cap(void);
extern void test_sink_tester_process_packet_invalid_source_cap(void);
extern void test_sink_tester_process_packet_not_source_cap(void);
extern void test_sink_tester_process_packet_invalid(void);
extern void test_crc32_goodcrc(void);
extern void test_crc32_source_capabilities(void);

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_decode_goodcrc);
    RUN_TEST(test_encode_goodcrc);
    RUN_TEST(test_pd_build_request_header);
    RUN_TEST(test_pd_build_request_header_no_data);
    RUN_TEST(test_pinger_process_packet_pong);
    RUN_TEST(test_pinger_process_packet_not_pong);
    RUN_TEST(test_pinger_process_packet_invalid);
    RUN_TEST(test_ponger_process_packet_ping);
    RUN_TEST(test_ponger_process_packet_not_ping);
    RUN_TEST(test_ponger_process_packet_invalid);
    RUN_TEST(test_source_emulator_prepare_packet);
    RUN_TEST(test_sink_tester_process_packet_valid_source_cap);
    RUN_TEST(test_sink_tester_process_packet_invalid_source_cap);
    RUN_TEST(test_sink_tester_process_packet_not_source_cap);
    RUN_TEST(test_sink_tester_process_packet_invalid);
    RUN_TEST(test_crc32_goodcrc);
    RUN_TEST(test_crc32_source_capabilities);
    return UNITY_END();
}
