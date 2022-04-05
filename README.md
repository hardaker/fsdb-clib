# FSDB C-library

This is a library for reading
[FSDB](https://www.isi.edu/~johnh/SOFTWARE/FSDB/) formatted datasets
(which is a typically tab-separated format containing additional
information, such as types and commands used to execute the code).

Other languages:

* Python: [pyfsdb](https://github.com/gawseed/pyfsdb)
* Perl: [FSDB](https://www.isi.edu/~johnh/SOFTWARE/FSDB/)
* Go: [go FSDB](https://www.isi.edu/~johnh/SOFTWARE/FSDB_GO/index.html)

## Building

To build the library, run `make` which will build *libfsdb.a* and test
it against a test binary in *tests/*

Automated installation: TBD

# Using it

Consider an example *test.fsdb* files as follows:

``` text
#fsdb -F s column_one:s column_two:l
life 42
universe 42000
```

An example file that opens this *test.fsdb* for reading should look
something like this:

``` c
#include <stdio.h>
#include <fsdb.h>

int main(int argc, char **argv) {
    FSDB *fsdb = fsdb_create_context();
    FILE *file_handle = fopen(filename, "r");
    int result;

    fsdb->save_rows = FSDB_TRUE;
    
    result = fsdb_parse_file_header(file_handle, fsdb);
    assert (result == FSDB_NO_ERROR);
```

Once opened, the file can be loaded using `fsdb_parse_file_contents`
(note as implemented today, this reads the entire file contents into memory):

``` c
    result = fsdb_parse_file_contents(file_handle, fsdb);
    assert (result == FSDB_NO_ERROR);
```

Once in memory, data can be best accessed with the FSDB_DATA macro,
which will return a pointer to the proper `fsdb_data` structure
containing the data:

``` c
    printf("row 0 column 1 is a long: %d\n", 
           FSDB_DATA(fsdb, 0, 1).data.v_long);
    printf("row 1 column 0 is a string: %s\n", 
           FSDB_DATA(fsdb, 1, 0).data.v_string);

}

```

## The *fsdb_data* structure

The *fsdb_data* structure contains both a copy of the raw data of the
column in `raw_string`, and a union of converted types in the `data`
struct field:

``` c
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
```

## The *FSDB* pointer

The *FSDB* structure itself contains a list of data (but please use the
*FSDB_DATA* macro for accessing in case storage changes in the
future), a list of *data_types* (e.g. *FSDB_TYPE_U_LONG*), and a list of
column names and other information:

``` c
typedef struct fsdb {
   /* public configuration */
   FSDB_BOOL save_rows;
   FSDB_TYPE_TYPE *data_types; /* Allocated array of types */

   /* public outputs */
   char *separator;
   char *header;
   char **columns; /* note: pointers to sections of _header_tokens */
   size_t columns_len;

   fsdb_data *rows;   /* note: alloc = [rows_len][columns_len] */
   size_t rows_len;
   char **row_string;  /* note: alloc = [rows_len] of duplicated rows */

   /* internal */
   char   *_header_tokens;
   size_t  _rows_allocated;
} FSDB;
```

## Finding column names

Since columns may move around in the input data, flexible code should
use the *fsdb_get_column_number()* function to retrieve the column
number of a given column name:

``` c
    int colnum = fsdb_get_column_number(fsdb, "column_one");
```
    
## Iteration example:

``` c
    int coltwo = fsdb_get_column_number(fsdb, "column_two");
    int i;
    long total = 0;

    /* check that it's the data type we expect */
    assert(fsdb->data_types[coltwo] == FSDB_TYPE_LONG);

    /* sum all the values */
    for (i = 0; i < fsdb->rows_len; i++) {
        total += FSDB_DATA(fsdb, i, coltwo).data.v_long;
    }
```

## Cleaning up

To free the associated memory:

``` c
    fsdb_free_context(fsdb);
```
