#ifndef FSDB_H
#define FSDB_H

typedef struct fsdb {
   char *separator;
   char *header;
} FSDB;

#define FSDB_NO_ERROR 0

/*
 * API
 */

int fsdb_parse_header(const char *header);

#endif
