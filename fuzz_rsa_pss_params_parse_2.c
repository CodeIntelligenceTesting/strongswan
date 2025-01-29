#include <assert.h>
#include <cifuzz/cifuzz.h>
#include "src/libstrongswan/credentials/keys/signature_params.h"
#include <asn1/oid.h>
#include <asn1/asn1_parser.h>

FUZZ_TEST_SETUP() {
    library_init(NULL, "fuzzing");
}

FUZZ_TEST(const uint8_t *data, size_t size) {
    if (size < sizeof(chunk_t)) {
        return;
    }

    chunk_t asn1 = { .ptr = (uint8_t*)data, .len = size };
    signature_params_t params = {0};
    if (signature_params_parse(asn1, 0, &params)) {
        // Simulate additional operations on parsed parameters
        signature_params_t *clone = signature_params_clone(&params);
        signature_params_destroy(clone);
        signature_params_destroy(&params);
    }
}
