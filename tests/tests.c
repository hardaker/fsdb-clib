#include <stdio.h>
#include <assert.h>

#include "fsdb.h"

void test_struct_contents() {
    struct fsdb s;
    s.separator = " ";
    s.header = "#fsdb -F s";
}

void test_header_parsing() {
    struct fsdb s;
    int result;

    result = fsdb_parse_header("#fsdb -F t one two three");
    assert(result == FSDB_NO_ERROR);
}

int main(int argc, char **argv) {
    test_struct_contents();
    test_header_parsing();
    
    printf("\nALL TESTS PASSED\n\n");
}

