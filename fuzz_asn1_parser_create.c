#include <assert.h>
#include <cifuzz/cifuzz.h>
#include <asn1/asn1_parser.h>
#include <utils/chunk.h>

// Define the asn1_test_t structure
typedef struct {
    bool success;
    int count;
    chunk_t blob;
} asn1_test_t;

// Define the octetStringObjects array
static const asn1Object_t octetStringObjects[] = {
    { 0, "octetString", ASN1_OCTET_STRING, ASN1_BODY },
    { 0, "exit", ASN1_EOC, ASN1_EXIT }
};

// Utility function copied from the test file
static void run_parser_test(const asn1Object_t *objects, int id, asn1_test_t *test)
{
    asn1_parser_t *parser;
    chunk_t object;
    int objectID, count = 0;
    bool success;

    parser = asn1_parser_create(objects, test->blob);
    while (parser->iterate(parser, &objectID, &object))
    {
        if (objectID == id)
        {
            count++;
        }
    }
    success = parser->success(parser);
    parser->destroy(parser);
}

FUZZ_TEST_SETUP() {
    // No setup needed
}

FUZZ_TEST(const uint8_t *data, size_t size) {
    if (size < 1) return;

    // Use the raw data to create a chunk
    chunk_t blob = { (uint8_t*)data, size };

    // Create a test case
    asn1_test_t test = { true, 1, blob };

    // Run the parser test with example objects
    run_parser_test(octetStringObjects, 0, &test);
}
