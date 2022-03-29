#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "fsdb.h"

void test_struct_contents() {
    struct fsdb s;
    s.separator = " ";
    s.header = "#fsdb -F s";
}

void test_column_names(FSDB *s) {
    /* check column name parsing too */
    assert(s->columns_len == 3);
    assert(strcmp(s->columns[0], "one") == 0);
    assert(strcmp(s->columns[1], "two") == 0);
    assert(strcmp(s->columns[2], "three") == 0);
    assert(s->columns[3] == NULL);
}

void test_header_parsing() {
    FSDB *s;
    int result;

    s = fsdb_create_context();

    /* a broken header */
    result = fsdb_parse_header(s, "#xfsdb -F t one two three", strlen("#xfsdb -F t one two three"));
    assert(result == FSDB_INVALID_HEADER);

    /* a good header */
    result = fsdb_parse_header(s, "#fsdb -F t one two three", strlen("#fsdb -F t one two three"));
    assert(result == FSDB_NO_ERROR);
    assert(s->separator != NULL);
    assert(strncmp(s->separator, "\t", 1) == 0);
    assert(strncmp(s->header, "#fsdb -F t one two three", strlen("#fsdb -F t one two three") )== 0);

    result = fsdb_parse_header(s, "#fsdb -F s one two three", strlen("#fsdb -F t one two three"));
    assert(result == FSDB_NO_ERROR);
    assert(s->separator != NULL);
    assert(strncmp(s->separator, " ", 1) == 0);
    assert(strncmp(s->header, "#fsdb -F s one two three", strlen("#fsdb -F s one two three") )== 0);

    result = fsdb_parse_header(s, "#fsdb -F S one two three", strlen("#fsdb -F t one two three"));
    assert(result == FSDB_NO_ERROR);
    assert(s->separator != NULL);
    assert(strncmp(s->header, "#fsdb -F S one two three", strlen("#fsdb -F S one two three") )== 0);
    assert(strncmp(s->separator, "  ", 1) == 0);

    test_column_names(s);

    fsdb_free_context(s);
}

void test_common_data_file(char *filename, FSDB_TYPE_TYPE col3_data_type) {
    FSDB *s;
    int result;
    FILE *fh;

    s = fsdb_create_context();
    s->save_rows = FSDB_TRUE;
    fh = fopen(filename, "r");

    result = fsdb_parse_file_header(fh, s);
    assert (result == FSDB_NO_ERROR);

    s->data_types[2] = col3_data_type;

    result = fsdb_parse_file_contents(fh, s);
    assert (result == FSDB_NO_ERROR);

    fprintf(stderr, "%s parsed files: rows=%d cols=%d\n", filename, s->rows_len, s->columns_len);

    result = FSDB_ROW_INDEX(s, 0, 0);
    assert(result == 0);
    result = FSDB_ROW_INDEX(s, 0, 2);
    assert(result == 2);
    result = FSDB_ROW_INDEX(s, 1, 1);
    fprintf(stderr, "result: %d\n", result);
    assert(result == 4);

    assert(fsdb_get_column_number(s, "one") == 0);
    assert(fsdb_get_column_number(s, "two") == 1);
    assert(fsdb_get_column_number(s, "three") == 2);

    test_column_names(s);
    assert(s->rows_len == 3);
    assert(s->columns_len == 3);
    /* we always treat column 3 as raw_strings, since we convert it sometimes */
    assert(strcmp(s->rows[0].raw_string, "a") == 0);
    assert(strcmp(s->rows[0].data.v_string, "a") == 0);
    assert(strcmp(s->rows[1].data.v_string, "b") == 0);
    assert(strcmp(s->rows[2].raw_string, "4") == 0);
    assert(strcmp(s->rows[3].data.v_string, "d") == 0);
    assert(strcmp(s->rows[3].raw_string, "d") == 0);
    {
        char *val = FSDB_COL(s, 1, 1).data.v_string;
        assert(strcmp(val, "e") == 0);
    }
    assert(strcmp(FSDB_COL(s, 2, 2).raw_string, "3") == 0);
    assert(s->rows[9].data.v_string == 0);
    assert(s->rows[9].raw_string == 0);

    /* check type conversions if specified */
    switch(s->data_types[2]) {
    case FSDB_TYPE_INT:
        assert(FSDB_COL(s, 2, 2).data.v_integer == 3);
        break;
    case FSDB_TYPE_DOUBLE:
        assert(FSDB_COL(s, 2, 2).data.v_double == 3.0);
        break;
    default:
        assert(strcmp(FSDB_COL(s, 2, 2).data.v_string, "3") == 0);
        break;
    } 

    fsdb_free_context(s);
}

void test_file_parsing() {
    FSDB *s;
    int result;
    FILE *fh;

    /* parse a file with comments and blanks  */
    test_common_data_file("testdata/test1.fsdb", FSDB_TYPE_STRING);
    test_common_data_file("testdata/test2-comments-blanks.fsdb", FSDB_TYPE_STRING);
    test_common_data_file("testdata/test3-tabs.fsdb", FSDB_TYPE_STRING);
    //test_common_data_file("testdata/test4-doublespaces.fsdb");

    // try test3 again but with data type conversion
    test_common_data_file("testdata/test3-tabs.fsdb", FSDB_TYPE_INT);
    test_common_data_file("testdata/test3-tabs.fsdb", FSDB_TYPE_DOUBLE);
}

int main(int argc, char **argv) {
    test_struct_contents();
    test_header_parsing();
    test_file_parsing();
    
    printf("\nALL TESTS PASSED\n\n");
}

