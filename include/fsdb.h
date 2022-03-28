#ifndef FSDB_H
#define FSDB_H

#define FSDB_BOOL  char
#define FSDB_FALSE 0
#define FSDB_TRUE  1

/* note: these macros are zero-indexed like the arrays */
#define FSDB_ROW_INDEX(s, row, col) ((s->columns_len * row) + col)
#define FSDB_COL(s, row, col) (s->rows[s->columns_len * row + col])

typedef union {
   int             v_integer;
   unsigned int    v_u_integer;
   long            v_long;
   unsigned long   v_u_long;
   float           v_float;
   double          v_double;
   char *          v_alloc_string;
} fsdb_data;
   

typedef struct fsdb {
   /* public configuration */
   FSDB_BOOL save_rows;

   /* public outputs */
   char *separator;
   char *header;
   char **columns; /* note: pointers to sections of _header_tokens */
   size_t columns_len;

   fsdb_data *rows;  /* note: alloc = [rows_len][columns_len] */
   size_t rows_len;

   /* internal */
   char   *_header_tokens;
   size_t  _rows_allocated;
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
int fsdb_parse_row(FSDB *s, char *line);

#endif
