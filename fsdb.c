#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsdb.h"

#define FIELD_FLAG 'F'

#define debug(x) printf(x);

FSDB *fsdb_create_context() {
    FSDB *s;
    s = calloc(sizeof(FSDB), 1);
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

    /* duplicate the header so we can use strtok on non-const */
    tmpbuf = strndup(header, header_len);

    entry = strtok_r(tmpbuf, " ", &tok_ptr);
    if (strlen(entry) != 5)
        return FSDB_INVALID_HEADER;
    if (! strncmp(entry, "#fsdb", 5) == 0)
        return FSDB_INVALID_HEADER;
    
    entry = strtok_r(NULL, " ", &tok_ptr);

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
                s->separator = "\t";
                break;
            case 's':
                s->separator = " ";
                break;
            case 'S':
                s->separator = "  ";
                break;
            default:
                debug("invalid separator type character\n");
                return FSDB_INVALID_HEADER; /* TODO: clean up mallocs */
            }

            break;

        default: /* column name */
            /* TODO: implement realloc when out of room */
            column_list[current_column++] = strdup(entry);
            break;
        }

        entry = strtok_r(NULL, " ", &tok_ptr);
    }

    free(tmpbuf);
    return FSDB_NO_ERROR;
}
