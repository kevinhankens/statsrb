#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static VALUE statsr_query(VALUE self, VALUE logfile, VALUE query_ns, VALUE query_limit, VALUE query_start, VALUE query_end) {
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
  int qstart = NUM2INT(query_start);
  int qend = NUM2INT(query_end);

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

      if ((qstart == 0 || statsr_ts >= qstart) && (qend == 0 || statsr_ts <= qend)) {
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
static VALUE statsr_write(VALUE self, VALUE logfile, VALUE mode) {
  FILE * file;
  const char *filepath = RSTRING_PTR(logfile);
  const char *filemode = RSTRING_PTR(mode);
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

  file = fopen (filepath, filemode);
  if (file==NULL) {
    fputs ("File error",stderr);
    exit (1);
  }

  // Iterate through the data array, writing the data as we go.
  for (i = 0; i < data_length; i++) {
    // @TODO make sure that these values are not empty before writing.
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
 * A method to split unique namespaces from internal memory and write them to individual files.
 */
static VALUE statsr_splitwrite(VALUE self, VALUE logdir, VALUE mode) {
  VALUE statsr_data = rb_iv_get(self, "@data");
  int len = RARRAY_LEN(statsr_data);
  int i, ii, ns_len;

  VALUE statsr_key_ts = rb_str_intern(rb_str_new2("ts"));
  VALUE statsr_key_ns = rb_str_intern(rb_str_new2("ns"));
  VALUE statsr_key_v = rb_str_intern(rb_str_new2("v"));

  VALUE ns_list = rb_ary_new();

  for (i = 0; i < len; i++) {
    if (!rb_ary_includes(ns_list, rb_hash_aref(rb_ary_entry(statsr_data, i), statsr_key_ns))) {
      rb_ary_push(ns_list, rb_hash_aref(rb_ary_entry(statsr_data, i), statsr_key_ns));
    }
  }

  ns_len = RARRAY_LEN(ns_list);

  for (i = 0; i < ns_len; i++) {
    VALUE tmp = rb_obj_dup(self);
    VALUE tmp_data = rb_ary_new();
    for (ii = 0; ii < len; ii++) {
      if (rb_str_cmp(rb_ary_entry(ns_list, i), rb_hash_aref(rb_ary_entry(statsr_data, ii), statsr_key_ns)) == 0) {
        rb_ary_push(tmp_data, rb_ary_entry(statsr_data, ii));
      }
    }
    //fputs (RSTRING_PTR(rb_obj_as_string(INT2NUM(RARRAY_LEN(tmp_data)))),stdout);
    rb_iv_set(tmp, "@data", tmp_data);

    // @todo, throw an exception if no trailing slash... or add one
    statsr_write(tmp, rb_str_plus(logdir, rb_ary_entry(ns_list, i)), mode);
  }

  return self;
}

/**
 * A method that is compatible with the rack api.
 * @TODO can we keep these in shared memory somehow and write in batches?
 */
static VALUE statsr_rack_call(VALUE self, VALUE env) {
  VALUE response = rb_ary_new();
  VALUE headers = rb_hash_new();
  VALUE body = rb_ary_new();
  VALUE statsr_data = rb_iv_get(self, "@data");
  VALUE statsr_hash = rb_hash_new();

  // Create symbols by passing ruby strings into rb_str_intern.
  VALUE statsr_key_ts = rb_str_intern(rb_str_new2("ts"));
  VALUE statsr_key_ns = rb_str_intern(rb_str_new2("ns"));
  VALUE statsr_key_v = rb_str_intern(rb_str_new2("v"));

  char *qs = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("PATH_INFO")));

  rb_hash_aset(headers, rb_str_new2("Content-Type"), rb_str_new2("text/html"));

  //const char *method = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("REQUEST_METHOD")));
  const char *method_get = "GET";
  const char *method_put = "PUT";
  // Remove the leading /
  qs++;
  const char *method = strtok(qs, "/\0");
  if (method && strcmp(method, method_put) == 0) {
    const char * statsr_str_ts = strtok(NULL, "/\0");
    long int statsr_ts = (statsr_str_ts) ? atoi(statsr_str_ts) : 0;
    if (statsr_ts == 0) {
      statsr_ts = (long int)time(NULL);
    }
    const char * statsr_str_ns = strtok(NULL, "/\0");
    if (statsr_str_ns) {
      VALUE statsr_ns = rb_str_new2(statsr_str_ns);
      const char * statsr_str_v = strtok(NULL, "/\0");
      int statsr_v = (statsr_str_v ) ? atoi(statsr_str_v ) : 0;

      // @TODO check for incorrect url format.
      rb_hash_aset(statsr_hash, statsr_key_ts, INT2NUM(statsr_ts));
      rb_hash_aset(statsr_hash, statsr_key_ns, statsr_ns);
      rb_hash_aset(statsr_hash, statsr_key_v, INT2NUM(statsr_v));
      rb_ary_push(statsr_data, statsr_hash);

      // @TODO move the log file to rack config.
      int data_length = RARRAY_LEN(statsr_data);
      if (data_length > 9) {
        statsr_sort(self);
        statsr_write(self, rb_str_new2("/tmp/ktest.log"), rb_str_new2("a+"));
        rb_ary_resize(statsr_data, 0);
      }

      rb_ary_push(body, statsr_ns);
    }
  }
  else if (method && strcmp(method, method_get) == 0) {
    const char * statsr_str_ns = strtok(NULL, "/\0");
    rb_ary_push(body, rb_str_new("[", 1));

    // If they didn't specify a namespace, bail out immediately.
    if (statsr_str_ns) {
      VALUE statsr_ns = rb_str_new2(statsr_str_ns);
      long int query_limit, query_start, query_end;
      const char * query_limit_str = strtok(NULL, "/\0");
      const char * query_start_str = strtok(NULL, "/\0");
      const char * query_end_str = strtok(NULL, "/\0");
      query_limit = (query_limit_str) ? atoi(query_limit_str) : 10;
      query_start = (query_start_str) ? atoi(query_start_str) : 0;
      query_end = (query_end_str) ? atoi(query_end_str) : 0;

      // Create a new Statsr object to query from.
      // @todo we probably need to assign a new array to @data to avoid messing up the pointers.
      VALUE tmp = rb_obj_dup(self);
      statsr_query(tmp, rb_str_new2("/tmp/ktest.log"), statsr_ns, INT2NUM(query_limit), INT2NUM(query_start), INT2NUM(query_end));
      statsr_sort(tmp);
      VALUE tmp_data = rb_iv_get(tmp, "@data");

      int data_length = RARRAY_LEN(tmp_data);
      int i;
   
      for (i = 0; i < data_length; i++) {
        rb_ary_push(body, rb_str_new("[", 1));
        rb_ary_push(body, rb_obj_as_string(rb_hash_aref(rb_ary_entry(tmp_data, i), statsr_key_ts )));
        rb_ary_push(body, rb_str_new(",'", 2));
        rb_ary_push(body, rb_hash_aref(rb_ary_entry(tmp_data, i), statsr_key_ns ));
        rb_ary_push(body, rb_str_new("',", 2));
        rb_ary_push(body, rb_obj_as_string(rb_hash_aref(rb_ary_entry(tmp_data, i), statsr_key_v )));
        rb_ary_push(body, rb_str_new("],\n", 3));
      } 
      rb_ary_resize(tmp_data, 0);
    }
    rb_ary_push(body, rb_str_new("]", 1));
  }
  else {
    rb_ary_push(response, INT2NUM(404));
    rb_ary_push(response, headers);
    rb_ary_push(response, body);
    return response;
  }

  rb_ary_push(response, INT2NUM(200));
  rb_ary_push(response, headers);
  rb_ary_push(response, body);

  return response;
}

/**
 * Class constructor, sets up an instance variable.
 * @TODO move symbol defs to constructor.
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
  rb_define_method(klass, "query", statsr_query, 5);
  rb_define_method(klass, "sort", statsr_sort, 0);
  rb_define_method(klass, "write", statsr_write, 2);
  rb_define_method(klass, "splitwrite", statsr_splitwrite, 2);
  rb_define_method(klass, "call", statsr_rack_call, 1);
  // Define :attr_accessor (read/write instance var)
  // Note that this must correspond with a call to rb_iv_self() and it's string name must be @data.
  rb_define_attr(klass, "data", 1, 1);
}
