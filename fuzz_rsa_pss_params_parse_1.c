#include <assert.h>
#include <cifuzz/cifuzz.h>
#include <stdlib.h>
#include "asn1/oid.h"
#include "asn1/asn1_parser.h"
#include "credentials/keys/signature_params.h"

// FUZZ_TEST_SETUP is used for any one-time initializations
FUZZ_TEST_SETUP() {
    // No specific setup required for this test
}

// FUZZ_TEST is the entry point for the fuzzing harness
FUZZ_TEST(const uint8_t *data, size_t size) {
    if (size < sizeof(chunk_t) + sizeof(int)) {
        return; // Ensure there is enough data
    }

    chunk_t asn1 = { .ptr = (uint8_t *)data, .len = size - sizeof(int) };
    signature_params_t params;
    int level0 = *(int *)(data + size - sizeof(int)); // Use last bytes for level0

    // Call the function to be fuzzed
    if (!signature_params_parse(asn1, level0, &params)) {
        return; // Handle parsing failure
    }

    // Clean up any allocated resources
    if (params.scheme == SIGN_RSA_EMSA_PSS && params.params) {
        free(params.params);
    }
}
