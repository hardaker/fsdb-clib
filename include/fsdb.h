#ifndef FSDB_H
#define FSDB_H

typedef struct fsdb {
   /* public */
   char *separator;
   char *header;
   char **columns; /* note: pointers to sections of _header_tokens */
   size_t columns_len;

   size_t rows_len;

   /* internal */
   char *_header_tokens;

} FSDB;

#define FSDB_NO_ERROR 0
#define FSDB_INVALID_HEADER 1
#define FSDB_INVALID_FILE 2

/*
 * API
 */

FSDB *fsdb_create_context();
void fsdb_free_context(FSDB *context);
int fsdb_parse_file(FILE *fh, FSDB *s);
int fsdb_parse_header(FSDB *s, const char *header, size_t header_len);
int fsdb_parse_row(FSDB *s, const char *line);

#endif
