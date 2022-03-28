#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsdb.h"

#define FIELD_FLAG 'F'
#define BUF_SIZE 1024*16

#define SAFEFREE(x) { if (x) { free(x); } }

#define debug(x) printf(x);

FSDB *fsdb_create_context() {
    FSDB *s;
    s = calloc(sizeof(FSDB), 1);
}

static void fsdb_free_internals(FSDB *s) {
    SAFEFREE(s->separator);
    SAFEFREE(s->header);
    SAFEFREE(s->_header_tokens);
    SAFEFREE(s->columns);
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
    char *tmpbuf = NULL;

    size_t column_list_len = 16;
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
    s->columns_len = 0;

    while(entry) {
        switch (entry[0]) {
        case '-': /* flag */
            if (strlen(entry) != 2) {
                debug("invalid flag len\n");
                return FSDB_INVALID_HEADER; /* TODO: clean up mallocs */
            }

            if (entry[1] != FIELD_FLAG) {
                debug("invalid flag value\n");
                return FSDB_INVALID_HEADER;
            }

            /* get the next token, which is the separator type*/
            entry = strtok_r(NULL, " ", &tok_ptr);

            if (strlen(entry) != 1) {
                debug("invalid separator type\n");
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
                debug("invalid separator type character\n");
                return FSDB_INVALID_HEADER; /* TODO: clean up mallocs */
            }

            break;

        default: /* column name */
            /* TODO: implement realloc when out of room */
            s->columns_len++;
            column_list[current_column++] = strdup(entry);
            break;
        }

        entry = strtok_r(NULL, " ", &tok_ptr);
    }

    return FSDB_NO_ERROR;
}

int fsdb_parse_row(FSDB *s, const char *row) {
    
}

int fsdb_parse_file(FILE *fh, FSDB *s) {
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
        

    while (cp = fgets(row_buffer, sizeof(row_buffer), fh)) {
        fsdb_parse_row(s, row_buffer);
    }
    return FSDB_NO_ERROR;
}
