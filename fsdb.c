#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsdb.h"

#define FIELD_FLAG 'F'
#define BUF_SIZE 1024*16

#define SAFEFREE(x) { if (x) { free(x); x = NULL; } }

// #define DEBUG(x) fprintf(stderr, x);
#define DEBUG(x)

FSDB *fsdb_create_context() {
    FSDB *s;
    s = calloc(sizeof(FSDB), 1);
    return s;
}

static void fsdb_free_internals(FSDB *s) {
    SAFEFREE(s->separator);
    SAFEFREE(s->header);
    SAFEFREE(s->_header_tokens);
    SAFEFREE(s->columns);
    SAFEFREE(s->data_types);

    if (s->row_string) {
        for(int i = 0; i < s->rows_len; i++) {
            SAFEFREE(s->row_string[i]);
        }
    }
    SAFEFREE(s->rows);

    s->columns_len = 0;
    s->rows_len = 0;
}

void fsdb_free_context(FSDB *s) {

    if (!s)
        return;
    fsdb_free_internals(s);
    SAFEFREE(s);
}

int fsdb_parse_header(FSDB *s, const char *header, size_t header_len) {
    char *tok_ptr = NULL;
    char *entry   = NULL;
    char *type_ptr = NULL;

    /* TODO: handle realloc of this */
    size_t column_list_len = 64;
    char **column_list = calloc(sizeof(char *), column_list_len);
    unsigned int current_column = 0;

    /* ensure basic header starts */
    if (header_len < 6)
        return FSDB_INVALID_HEADER;
    if (! strncmp(header, "#fsdb ", 6) == 0)
        return FSDB_INVALID_HEADER;

    /* duplicate the token_headers so we can use strtok on non-const */
    fsdb_free_internals(s);
    s->header = strndup(header, header_len);
    s->_header_tokens = strndup(header, header_len);

    entry = strtok_r(s->_header_tokens, " ", &tok_ptr);
    if (strlen(entry) != 5)
        return FSDB_INVALID_HEADER;
    if (! strncmp(entry, "#fsdb", 5) == 0)
        return FSDB_INVALID_HEADER;
    
    entry = strtok_r(NULL, " ", &tok_ptr);
    s->columns = column_list;
    s->data_types = calloc(sizeof(FSDB_TYPE_TYPE), column_list_len);
    s->columns_len = 0;

    while(entry) {
        switch (entry[0]) {
        case '-': /* flag */
            if (strlen(entry) != 2) {
                DEBUG("invalid flag len\n");
                return FSDB_INVALID_HEADER; /* TODO: clean up mallocs */
            }

            if (entry[1] != FIELD_FLAG) {
                DEBUG("invalid flag value\n");
                return FSDB_INVALID_HEADER;
            }

            /* get the next token, which is the separator type*/
            entry = strtok_r(NULL, " ", &tok_ptr);

            if (strlen(entry) != 1) {
                DEBUG("invalid separator type\n");
                return FSDB_INVALID_HEADER; /* TODO: clean up mallocs */
            }

            switch(entry[0]) {
            case 't':
                s->separator = strdup("\t");
                break;
            case 's':
                s->separator = strdup(" ");
                break;
            case 'S':
                s->separator = strdup("  ");
                break;
            default:
                DEBUG("invalid separator type character\n");
                return FSDB_INVALID_HEADER; /* TODO: clean up mallocs */
            }

            break;

        default: /* column name */
            /* TODO: implement realloc when out of room */
            s->columns_len++;
            column_list[current_column] = strdup(entry);

            /* check if the type is specified */
            if ((type_ptr = index(column_list[current_column], ':'))) {

                *type_ptr = '\0';

                switch(*(type_ptr+1)) {
                case 'f':
                case 'd':
                    s->data_types[current_column] = FSDB_TYPE_DOUBLE;
                    break;
                case 'l':
                case 'i':
                    s->data_types[current_column] = FSDB_TYPE_LONG;
                    break;
                case 'L':
                case 'I':
                    s->data_types[current_column] = FSDB_TYPE_U_LONG;
                    break;

                case 's':
                    s->data_types[current_column] = FSDB_TYPE_STRING;
                    break;

                default:
                    fprintf(stderr, "FSDB warning, unknown type: %c for column %s\n",
                            *(type_ptr+1), entry);
                }
            }

            current_column++;
            break;
        }

        entry = strtok_r(NULL, " ", &tok_ptr);
    }

    return FSDB_NO_ERROR;
}

int fsdb_parse_row(FSDB *s, char *row) {
    char *tok_ptr = NULL;
    char *entry = NULL;
    int i = 0;

    /* skip blank lines */
    if (strlen(row) == 0)
        return FSDB_NO_ERROR;

    /* skip comments */
    if (row[0] == '#')
        return FSDB_NO_ERROR;

    s->rows_len++;
    // fprintf(stderr, "counting lines: %d: %s\n", s->rows_len, row);

    if (s->save_rows) {
        if (s->_rows_allocated == 0) {
            s->_rows_allocated = 4096;
            /* allocate a large chunk of memory that can store everything */
            s->rows = calloc(sizeof(fsdb_data) * s->_rows_allocated * s->columns_len, 1);
            s->row_string = calloc(sizeof(char *), s->_rows_allocated);
            //fprintf(stderr, "allocated: %zu rows\n", s->_rows_allocated);
        } else if (s->rows_len > s->_rows_allocated) {
            s->_rows_allocated *= 2;
            s->rows = realloc(s->rows, sizeof(fsdb_data) * s->_rows_allocated * s->columns_len);
            s->row_string = realloc(s->row_string, sizeof(char *) * s->_rows_allocated);
            //fprintf(stderr, "reallocated: %zu rows\n", s->_rows_allocated);
        }

        s->row_string[s->rows_len-1] = strdup(row);

        entry = strtok_r(s->row_string[s->rows_len-1], s->separator, &tok_ptr);
        for(i = 0; entry && i < s->columns_len; i++) {
            FSDB_DATA(s, s->rows_len-1, i).raw_string = entry;
            switch(s->data_types[i]) {
            case FSDB_TYPE_U_LONG:
                FSDB_DATA(s, s->rows_len-1, i).data.v_u_long = atoi(entry);
                break;
            case FSDB_TYPE_LONG:
                FSDB_DATA(s, s->rows_len-1, i).data.v_long = atoi(entry);
                break;
            case FSDB_TYPE_DOUBLE:
                FSDB_DATA(s, s->rows_len-1, i).data.v_double = atof(entry);
                break;
            default:
                FSDB_DATA(s, s->rows_len-1, i).data.v_string = entry;
                break;
            }
            entry = strtok_r(NULL, s->separator, &tok_ptr);
        }
    }
    
    // TODO: need a callback routine so saving isn't required

    return  FSDB_NO_ERROR;
}

int fsdb_parse_file_header(FILE *fh, FSDB *s) {
    char row_buffer[BUF_SIZE];
    char *cp;
    int ret_code;

    if (!fh) {
        return FSDB_INVALID_FILE;
    }

    /* get the header */
    cp = fgets(row_buffer, sizeof(row_buffer), fh);
    if (!cp) {
        return FSDB_INVALID_FILE;
    }

    row_buffer[strlen(row_buffer)-1] = '\0'; /* drop newline */
    ret_code = fsdb_parse_header(s, row_buffer, sizeof(row_buffer));
    if (ret_code != FSDB_NO_ERROR) {
        return ret_code;
    }
    return FSDB_NO_ERROR;
}

int fsdb_parse_file_contents(FILE *fh, FSDB *s) {
    char row_buffer[BUF_SIZE];
    char *cp;

    while ((cp = fgets(row_buffer, sizeof(row_buffer), fh))) {
        row_buffer[strlen(row_buffer)-1] = '\0'; /* drop newline */
        fsdb_parse_row(s, row_buffer);
    }
    return FSDB_NO_ERROR;
}

int fsdb_parse_file(FILE *fh, FSDB *s) {
    int ret_code;

    ret_code = fsdb_parse_file_header(fh, s);
    if (ret_code)
        return ret_code;

    return fsdb_parse_file_contents(fh, s);
}

int fsdb_get_column_number(FSDB *s, const char *column_name) {
    if (s->columns_len == 0 || !s->columns) {
        return FSDB_NO_HEADER_INFORMATION;
    }
    for(int i = 0; i < s->columns_len; i++) {
        if (strcmp(s->columns[i], column_name) == 0) {
            return i;
        }
    }
    return FSDB_NO_SUCH_COLUMN;
}
