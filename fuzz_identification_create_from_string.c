#include <assert.h>
#include <cifuzz/cifuzz.h>
#include <library.h>
#include <utils/identification.h>
#include <networking/host.h>

FUZZ_TEST_SETUP() {
    library_init(NULL, "fuzz_test");
    atexit(library_deinit);
}

FUZZ_TEST(const uint8_t *data, size_t size) {
    if (size < 1) {
        return;
    }

    char *input = (char *)malloc(size + 1);
    if (!input) {
        return;
    }
    memcpy(input, data, size);
    input[size] = '\0';

    identification_t *id = identification_create_from_string(input);
    if (id) {
        chunk_t enc = id->get_encoding(id);
        for (size_t i = 0; i < enc.len; i++) {
            (void)enc.ptr[i];
        }
        id->destroy(id);
    }

    host_t *host = host_create_from_dns(input, AF_UNSPEC, 0);
    if (host) {
        host->destroy(host);
    }

    free(input);
}
