#ifndef FSDB_H
#define FSDB_H

typedef struct fsdb {
   char *separator;
   char *header;
} FSDB;

#define FSDB_NO_ERROR 0
#define FSDB_INVALID_HEADER 1

/*
 * API
 */

FSDB *fsdb_create_context();
int fsdb_parse_header(FSDB *s, const char *header, size_t header_len);

#endif
