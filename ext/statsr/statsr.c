#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static VALUE statsr_query(VALUE self, VALUE logfile, VALUE query_ns, int query_limit) {
  // File pointer.
  FILE * file;
  // File line max length.
  int line_size = 256;
  char *line = (char *) malloc(line_size);
  const char *filepath = RSTRING_PTR(logfile);
  const char *query_ns_char = RSTRING_PTR(query_ns);

  // Create symbols by passing ruby strings into rb_str_intern.
  VALUE statsr_key_ts = rb_str_intern(rb_str_new2("ts"));
  VALUE statsr_key_ns = rb_str_intern(rb_str_new2("ns"));
  VALUE statsr_key_v = rb_str_intern(rb_str_new2("v"));
  // Create an empty string for comparison.
  VALUE statsr_str_empty = rb_str_new2("");

  // Convert into an int that ruby understands.
  int limit = NUM2INT(query_limit);

  // Return array instantiation.
  VALUE statsr_data = rb_iv_get(self, "@data");
  // @TODO does this garbage collect all of the old hash data?
  rb_ary_resize(statsr_data, 0);

  file = fopen (filepath, "r");
  if (file==NULL) {
    fputs ("File error",stderr);
    exit (1);
  }

  int count = 0;

  while (NULL != fgets(line, line_size, file) && count < limit) {

    // strstr doesn't work with newline chars. 
    size_t len = strlen(line) - 1;
    if (line[len] == '\n');
        line[len] = '\0';

    // If the namespace is in the row, explode it.
    if (line[0] != '\0' && line[0] != '\n' && strchr(line, query_ns_char[0]) && strstr(line, query_ns_char)) {
      VALUE statsr_event = rb_hash_new();

      // I tried sscanf for convenience, but it was predictably slower.
      //int statsr_ts, statsr_v;
      //sscanf(line, "%d\t%*s\t%d", &statsr_ts, &statsr_v);

      // @TODO this should something more robust than atoi.
      int statsr_ts = atoi(strtok(line, "\t"));
      // @TODO this should probably use the actual namespace if we do wildcard queries.
      VALUE statsr_str_ns = rb_str_new2(strtok(NULL, "\t"));
      //strtok(NULL, "\t");
      int statsr_v = atoi(strtok(NULL, "\0"));

      // @TODO this should really query the namespace exactly instead of just relying on strstr.
      //if (rb_str_cmp(query_ns, statsr_str_empty) == 0 || rb_str_cmp(query_ns, statsr_str_ns) == 0) {
      if (statsr_ts && (statsr_v || statsr_v == 0)) {
        rb_hash_aset(statsr_event, statsr_key_ts, INT2NUM(statsr_ts));
        rb_hash_aset(statsr_event, statsr_key_ns, statsr_str_ns);
        //rb_hash_aset(statsr_event, statsr_key_ns, query_ns);
        rb_hash_aset(statsr_event, statsr_key_v, INT2NUM(statsr_v));
        rb_ary_push(statsr_data, statsr_event);
        count++;
      }
    }
  }

  // terminate
  fclose (file);
  free (line);

  //return statsr_data;
  //rb_iv_set(self, "@data", statsr_data);

  return self;
}

/**
 * Implementation of quicksort algorithm.
 */
void time_sort(int left, int right, VALUE ary, VALUE statsr_key_ts) {
  int i = left;
  int j = right;
  int p = (i + j) / 2;
  int pv = NUM2INT(rb_hash_aref(rb_ary_entry(ary, p), statsr_key_ts));
  VALUE tmp;

  while (i <= j) {
    while (NUM2INT(rb_hash_aref(rb_ary_entry(ary, i), statsr_key_ts)) < pv) {
      i++;
    }
    while (NUM2INT(rb_hash_aref(rb_ary_entry(ary, j), statsr_key_ts)) > pv) {
      j--;
    }
    if (i <= j) {
      tmp = rb_hash_aref(rb_ary_entry(ary, i), statsr_key_ts);
      rb_hash_aset(rb_ary_entry(ary, i), statsr_key_ts, rb_hash_aref(rb_ary_entry(ary, j), statsr_key_ts ));
      rb_hash_aset(rb_ary_entry(ary, j), statsr_key_ts, tmp);
      i++;
      j--;
    }
  }

  if (left < j) {
    time_sort(left, j, ary, statsr_key_ts);
  }
  if (i < right) {
    time_sort(i, right, ary, statsr_key_ts);
  }
}

static VALUE statsr_sort(VALUE self) {
  VALUE statsr_data = rb_iv_get(self, "@data");
  int len = RARRAY_LEN(statsr_data);
  VALUE statsr_key_ts = rb_str_intern(rb_str_new2("ts"));
  time_sort(0, len - 1, statsr_data, statsr_key_ts);
  return statsr_data;
}

/**
 * Write the in-memory data to a file.
 */
static VALUE statsr_write(VALUE self, VALUE logfile) {
  FILE * file;
  const char *filepath = RSTRING_PTR(logfile);
  VALUE statsr_data = rb_iv_get(self, "@data");
  int data_length = RARRAY_LEN(statsr_data);
  int i;
  int line_size = 256;
  int tmp_ts, tmp_v;
  const char *tmp_ns = (char *) malloc(line_size);
  
  // Create symbols by passing ruby strings into rb_str_intern.
  VALUE statsr_key_ts = rb_str_intern(rb_str_new2("ts"));
  VALUE statsr_key_ns = rb_str_intern(rb_str_new2("ns"));
  VALUE statsr_key_v = rb_str_intern(rb_str_new2("v"));

  file = fopen (filepath, "w+");
  if (file==NULL) {
    fputs ("File error",stderr);
    exit (1);
  }

  // Iterate through the data array, writing the data as we go.
  for (i = 0; i < data_length; i++) {
    //VALUE tmp_line = rb_str_tmp_new(line_size);
    tmp_ts = NUM2INT(rb_hash_aref(rb_ary_entry(statsr_data, i), statsr_key_ts));
    tmp_ns = RSTRING_PTR(rb_hash_aref(rb_ary_entry(statsr_data, i), statsr_key_ns));
    tmp_v = NUM2INT(rb_hash_aref(rb_ary_entry(statsr_data, i), statsr_key_v));
    fprintf(file, "%d\t%s\t%d\n", tmp_ts, tmp_ns, tmp_v);
    //rb_str_free(tmp_line);
  }

  fclose (file);
  return self;
}

/**
 * Class constructor, sets up an instance variable.
 */
static VALUE statsr_constructor(VALUE self) {
  VALUE statsr_data = rb_ary_new();
  rb_iv_set(self, "@data", statsr_data);

  return self;
}

/* ruby calls this to load the extension */
void Init_statsr(void) {
  VALUE klass = rb_define_class("Statsr", rb_cObject);

  rb_define_method(klass, "initialize", statsr_constructor, 0);
  rb_define_method(klass, "query", statsr_query, 3);
  rb_define_method(klass, "sort", statsr_sort, 0);
  rb_define_method(klass, "write", statsr_write, 1);
  // Define :attr_accessor (read/write instance var)
  // Note that this must correspond with a call to rb_iv_self() and it's string name must be @data.
  rb_define_attr(klass, "data", 1, 1);
}
