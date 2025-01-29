#include <assert.h>
#include <cifuzz/cifuzz.h>
#include "src/libstrongswan/credentials/keys/signature_params.h"
#include <asn1/oid.h>
#include <asn1/asn1_parser.h>

FUZZ_TEST_SETUP() {
    // Initialization tasks if needed
}

FUZZ_TEST(const uint8_t *data, size_t size) {
    if (size < sizeof(chunk_t) + sizeof(int)) {
        return;
    }

    chunk_t asn1;
    asn1.ptr = (uint8_t *)data;
    asn1.len = size - sizeof(int);

    signature_params_t params = {0};
    int level0 = *(int *)(data + size - sizeof(int));

    bool result = signature_params_parse(asn1, level0, &params);

    if (params.scheme == SIGN_RSA_EMSA_PSS && params.params) {
        free(params.params);
    }

    // No assertions or logging, just execute the function
    (void)result;
}
