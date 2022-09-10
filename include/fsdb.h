#ifndef FSDB_H
#define FSDB_H

#include <stdio.h>

#define FSDB_BOOL  char
#define FSDB_FALSE 0
#define FSDB_TRUE  1

/* default type is a raw string */
#define FSDB_TYPE_STRING 0
#define FSDB_TYPE_INT 1
#define FSDB_TYPE_U_INT 2
#define FSDB_TYPE_LONG 3
#define FSDB_TYPE_U_LONG 4
#define FSDB_TYPE_DOUBLE 5

#define FSDB_TYPE_TYPE char

#define FSDB_NO_ERROR 0
#define FSDB_INVALID_HEADER -1
#define FSDB_INVALID_FILE -2
#define FSDB_NO_HEADER_INFORMATION -3
#define FSDB_NO_SUCH_COLUMN -4


/* note: these macros are zero-indexed like the arrays */
#define FSDB_ROW_INDEX(__s, __n_row, __n_col) ((__s->columns_len * (__n_row)) + (__n_col))
/* FSDB_DATA(context, row_number, col_number) (where rows and columns are zero-indexed)*/
#define FSDB_DATA(__s, __n_row, __n_col) (__s->rows[__s->columns_len * (__n_row) + (__n_col)])
#define FSDB_RAW_DATA(__s, __n_row, __n_col) (__s->rows[__s->columns_len * (__n_row) + (__n_col)])

typedef union {
   int             v_integer;
   unsigned int    v_u_integer;
   long            v_long;
   unsigned long   v_u_long;
   float           v_float;
   double          v_double;
   char *          v_string;
} converted_fsdb_data;

typedef struct fsdb_data_s {
   converted_fsdb_data data;
   char *raw_string;
} fsdb_data;

typedef struct fsdb {
   /* public configuration */
   FSDB_BOOL save_rows;

   /* public FSDB header information */
   char *separator;
   char *header;       /* the saved full header string */
   char **columns;     /* note: pointers to sections of _header_tokens */
   size_t columns_len;
   FSDB_TYPE_TYPE *data_types; /* Allocated array of types */

   /* containers used when save_rows is true */
   size_t rows_len;
   fsdb_data *rows;    /* note: alloc size = [rows_len][columns_len] */
   char **row_string;  /* note: alloc size = [rows_len] of duplicated rows */

   /* internal */
   char   *_header_tokens;
   size_t  _rows_allocated;
} FSDB;

/*
 * API
 */

FSDB *fsdb_create_context();
void fsdb_free_context(FSDB *context);
int fsdb_parse_file(FILE *fh, FSDB *s);
int fsdb_parse_file_header(FILE *fh, FSDB *s);
int fsdb_parse_file_contents(FILE *fh, FSDB *s);
int fsdb_parse_header(FSDB *s, const char *header, size_t header_len);
int fsdb_parse_row(FSDB *s, char *line);
int fsdb_get_column_number(FSDB *s, const char *column_name);

#endif
