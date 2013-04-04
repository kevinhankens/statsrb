#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static VALUE statsrb_query(VALUE self, VALUE logfile, VALUE query_ns, VALUE query_limit, VALUE query_start, VALUE query_end) {
  // File pointer.
  FILE * file;
  // File line max length.
  int line_size = 256;
  char *line = (char *) malloc(line_size);
  const char *filepath = RSTRING_PTR(logfile);
  const char *query_ns_char = RSTRING_PTR(query_ns);

  // @data hash key symbols.
  VALUE statsrb_key_ts = rb_iv_get(self, "@key_ts");
  VALUE statsrb_key_ns = rb_iv_get(self, "@key_ns");
  VALUE statsrb_key_v = rb_iv_get(self, "@key_v");
  // Create an empty string for comparison.
  VALUE statsrb_str_empty = rb_str_new2("");

  // Convert into an int that ruby understands.
  int limit = NUM2INT(query_limit);
  int qstart = NUM2INT(query_start);
  int qend = NUM2INT(query_end);

  // Return array instantiation.
  VALUE statsrb_data = rb_iv_get(self, "@data");
  // @TODO does this garbage collect all of the old hash data?
  rb_ary_resize(statsrb_data, 0);

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
      VALUE statsrb_event = rb_hash_new();

      // I tried sscanf for convenience, but it was predictably slower.
      //int statsrb_ts, statsrb_v;
      //sscanf(line, "%d\t%*s\t%d", &statsrb_ts, &statsrb_v);

      // @TODO this should something more robust than atoi.
      int statsrb_ts = atoi(strtok(line, "\t"));

      if (statsrb_ts != NULL && (qstart == 0 || statsrb_ts >= qstart) && (qend == 0 || statsrb_ts <= qend)) {
        // @TODO this should probably use the actual namespace if we do wildcard queries.
        VALUE statsrb_str_ns = rb_str_new2(strtok(NULL, "\t"));
        //strtok(NULL, "\t");
        int statsrb_v = atoi(strtok(NULL, "\0"));
  
        // @TODO this should really query the namespace exactly instead of just relying on strstr.
        //if (rb_str_cmp(query_ns, statsrb_str_empty) == 0 || rb_str_cmp(query_ns, statsrb_str_ns) == 0) {
        if (statsrb_ts && (statsrb_v || statsrb_v == 0)) {
          rb_hash_aset(statsrb_event, statsrb_key_ts, INT2NUM(statsrb_ts));
          rb_hash_aset(statsrb_event, statsrb_key_ns, statsrb_str_ns);
          //rb_hash_aset(statsrb_event, statsrb_key_ns, query_ns);
          rb_hash_aset(statsrb_event, statsrb_key_v, INT2NUM(statsrb_v));
          rb_ary_push(statsrb_data, statsrb_event);
          count++;
        }
      }
    }
  }

  // terminate
  fclose (file);
  free (line);

  //return statsrb_data;
  //rb_iv_set(self, "@data", statsrb_data);

  return self;
}

/**
 * Implementation of quicksort algorithm.
 */
void time_sort(int left, int right, VALUE ary, VALUE statsrb_key_ts) {
  int i = left;
  int j = right;
  int p = (i + j) / 2;
  int pv = NUM2INT(rb_hash_aref(rb_ary_entry(ary, p), statsrb_key_ts));
  VALUE tmp;

  while (i <= j) {
    while (NUM2INT(rb_hash_aref(rb_ary_entry(ary, i), statsrb_key_ts)) < pv) {
      i++;
    }
    while (NUM2INT(rb_hash_aref(rb_ary_entry(ary, j), statsrb_key_ts)) > pv) {
      j--;
    }
    if (i <= j) {
      tmp = rb_ary_entry(ary, i);
      rb_ary_store(ary, i, rb_ary_entry(ary, j));
      rb_ary_store(ary, j, tmp);
      i++;
      j--;
    }
  }

  if (left < j) {
    time_sort(left, j, ary, statsrb_key_ts);
  }
  if (i < right) {
    time_sort(i, right, ary, statsrb_key_ts);
  }
}

/**
 * Sort the internal data using a quicksort algorithm based on the hash element's timestamp.
 */
static VALUE statsrb_sort(VALUE self) {
  VALUE statsrb_data = rb_iv_get(self, "@data");
  int len = RARRAY_LEN(statsrb_data);
  if (len > 0) {
    VALUE statsrb_key_ts = rb_iv_get(self, "@key_ts");
    time_sort(0, len - 1, statsrb_data, statsrb_key_ts);
  }
  return statsrb_data;
}

/**
 * Write the in-memory data to a file.
 */
static VALUE statsrb_write(VALUE self, VALUE logfile, VALUE mode) {
  FILE * file;
  const char *filepath = RSTRING_PTR(logfile);
  const char *filemode = RSTRING_PTR(mode);
  VALUE statsrb_data = rb_iv_get(self, "@data");
  int data_length = RARRAY_LEN(statsrb_data);
  int i;
  int line_size = 256;
  int tmp_ts, tmp_v;
  const char *tmp_ns = (char *) malloc(line_size);
  
  // @data hash key symbols.
  VALUE statsrb_key_ts = rb_iv_get(self, "@key_ts");
  VALUE statsrb_key_ns = rb_iv_get(self, "@key_ns");
  VALUE statsrb_key_v = rb_iv_get(self, "@key_v");

  file = fopen (filepath, filemode);
  if (file==NULL) {
    fputs ("File error",stderr);
    exit (1);
  }

  // Iterate through the data array, writing the data as we go.
  for (i = 0; i < data_length; i++) {
    // @TODO make sure that these values are not empty before writing.
    //VALUE tmp_line = rb_str_tmp_new(line_size);
    tmp_ts = NUM2INT(rb_hash_aref(rb_ary_entry(statsrb_data, i), statsrb_key_ts));
    tmp_ns = RSTRING_PTR(rb_hash_aref(rb_ary_entry(statsrb_data, i), statsrb_key_ns));
    tmp_v = NUM2INT(rb_hash_aref(rb_ary_entry(statsrb_data, i), statsrb_key_v));
    fprintf(file, "%d\t%s\t%d\n", tmp_ts, tmp_ns, tmp_v);
    //rb_str_free(tmp_line);
  }

  fclose (file);
  return self;
}

/**
 * A method to split unique namespaces from internal memory and write them to individual files.
 */
static VALUE statsrb_split_write(VALUE self, VALUE logdir, VALUE mode) {
  VALUE statsrb_data = rb_iv_get(self, "@data");
  int len = RARRAY_LEN(statsrb_data);
  int i, ii, ns_len;

  // @data hash key symbols.
  VALUE statsrb_key_ts = rb_iv_get(self, "@key_ts");
  VALUE statsrb_key_ns = rb_iv_get(self, "@key_ns");
  VALUE statsrb_key_v = rb_iv_get(self, "@key_v");

  VALUE ns_list = rb_ary_new();

  for (i = 0; i < len; i++) {
    if (!rb_ary_includes(ns_list, rb_hash_aref(rb_ary_entry(statsrb_data, i), statsrb_key_ns))) {
      rb_ary_push(ns_list, rb_hash_aref(rb_ary_entry(statsrb_data, i), statsrb_key_ns));
    }
  }

  ns_len = RARRAY_LEN(ns_list);

  for (i = 0; i < ns_len; i++) {
    VALUE tmp = rb_obj_dup(self);
    VALUE tmp_data = rb_ary_new();
    for (ii = 0; ii < len; ii++) {
      if (rb_str_cmp(rb_ary_entry(ns_list, i), rb_hash_aref(rb_ary_entry(statsrb_data, ii), statsrb_key_ns)) == 0) {
        rb_ary_push(tmp_data, rb_ary_entry(statsrb_data, ii));
      }
    }
    //fputs (RSTRING_PTR(rb_obj_as_string(INT2NUM(RARRAY_LEN(tmp_data)))),stderr);
    rb_iv_set(tmp, "@data", tmp_data);

    // @todo, throw an exception if no trailing slash... or add one
    statsrb_write(tmp, rb_str_plus(logdir, rb_ary_entry(ns_list, i)), mode);
  }

  return self;
}

/**
 * Parses the query string parameters.
 *
 * @param char * qs
 *   The location of the query string.
 *
 * @return VALUE
 *   The ruby hash containing the query string keys and values.
 */
static VALUE statsrb_parse_qs(char *qs) {
  char *qsk, *qsv;
  VALUE query_string_tmp = rb_ary_new();
  VALUE query_string = rb_hash_new();
  qsk = strtok(qs, "&\0");
  while (qsk != NULL) {
    rb_ary_push(query_string_tmp, rb_str_new2(qsk));
    qsk = strtok(NULL, "&\0");
  }
  int qslen = RARRAY_LEN(query_string_tmp);
  int qsi;
  for (qsi = 0; qsi < qslen; qsi++) {
    qsk = strtok(RSTRING_PTR(rb_ary_entry(query_string_tmp, qsi)), "=\0");
    qsv = strtok(NULL, "\0");
    if (qsv != NULL) {
      rb_hash_aset(query_string, rb_str_new2(qsk), rb_str_new2(qsv));
    }
    else if(qsk != NULL && qsv != NULL) {
      rb_hash_aset(query_string, rb_str_new2(qsk), rb_str_new2(""));
    }
  }

  return query_string;
}

/**
 * A method that is compatible with the rack api.
 * @TODO can we keep these in shared memory somehow and write in batches?
 */
static VALUE statsrb_rack_call(VALUE self, VALUE env) {
  VALUE response = rb_ary_new();
  VALUE headers = rb_hash_new();
  VALUE body = rb_ary_new();
  VALUE statsrb_data = rb_iv_get(self, "@data");
  VALUE statsrb_hash = rb_hash_new();

  // @data hash key symbols.
  VALUE statsrb_key_ts = rb_iv_get(self, "@key_ts");
  VALUE statsrb_key_ns = rb_iv_get(self, "@key_ns");
  VALUE statsrb_key_v = rb_iv_get(self, "@key_v");

  char *path = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("PATH_INFO")));

  rb_hash_aset(headers, rb_str_new2("Content-Type"), rb_str_new2("text/json"));

  // Parse the query string
  char *qs = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("QUERY_STRING")));
  VALUE query_string = statsrb_parse_qs(qs);

  //const char *method = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("REQUEST_METHOD")));
  // @TODO consider moving the request method to the proper REQUEST_METHOD
  const char *method_get = "GET";
  const char *method_put = "PUT";
  // Remove the leading /
  path++;
  const char *method = strtok(path, "/\0");
  if (method && strcmp(method, method_put) == 0) {
    long int statsrb_ts, statsrb_v;

    // Get the timestamp, default to now.
    VALUE statsrb_ts_qs = rb_hash_aref(query_string, rb_str_new("time", 4));
    if (statsrb_ts_qs != Qnil) {
      statsrb_ts = atoi(RSTRING_PTR(statsrb_ts_qs ));
    }
    else {
      statsrb_ts = (long int)time(NULL);
    }

    // Get the namespace.
    VALUE statsrb_ns = rb_hash_aref(query_string, rb_str_new("name", 4));
    if (statsrb_ns == Qnil) {
      statsrb_ns = NULL;
    }

    if (statsrb_ns) {
      // Get the value.
      statsrb_v= 0;
      VALUE statsrb_v_qs = rb_hash_aref(query_string, rb_str_new("value", 5));
      if (statsrb_v_qs != Qnil) {
        statsrb_v = atoi(RSTRING_PTR(statsrb_v_qs));
      }

      rb_hash_aset(statsrb_hash, statsrb_key_ts, INT2NUM(statsrb_ts));
      rb_hash_aset(statsrb_hash, statsrb_key_ns, statsrb_ns);
      rb_hash_aset(statsrb_hash, statsrb_key_v, INT2NUM(statsrb_v));
      rb_ary_push(statsrb_data, statsrb_hash);

      int data_length = RARRAY_LEN(statsrb_data);
      rb_ary_push(body, rb_obj_as_string(INT2NUM(RARRAY_LEN(statsrb_data))));
      if (data_length > 9) {
      rb_ary_push(body, rb_str_new2("split"));
        statsrb_sort(self);
        statsrb_split_write(self, rb_iv_get(self, "@split_file_dir"), rb_str_new2("a+"));
        rb_ary_resize(statsrb_data, 0);
      }

      rb_ary_push(body, statsrb_ns);
    }
  }
  else if (method && strcmp(method, method_get) == 0) {
    const char * statsrb_str_ns = strtok(NULL, "/\0");
    if (statsrb_str_ns == NULL) {
      statsrb_str_ns = "data";
    }

    VALUE jsoncallback = rb_hash_aref(query_string, rb_str_new("jsoncallback", 12));
    if (jsoncallback != Qnil) {
      rb_ary_push(body, rb_str_plus(jsoncallback, rb_str_new("(", 1)));
    }
    char json_start[256];
    sprintf(json_start, "{\"%s\":[", statsrb_str_ns);
    rb_ary_push(body, rb_str_new2(json_start));

    // If they didn't specify a namespace, bail out immediately.
    if (statsrb_str_ns) {
      VALUE statsrb_ns = rb_str_new2(statsrb_str_ns);
      long int query_limit, query_start, query_end;

      // Get the query limit.
      query_limit = 100;
      VALUE query_limit_qs = rb_hash_aref(query_string, rb_str_new("limit", 5));
      if (query_limit_qs != Qnil) {
        query_limit = atoi(RSTRING_PTR(query_limit_qs));
      }

      // Get the query start.
      query_start = 0;
      VALUE query_start_qs = rb_hash_aref(query_string, rb_str_new("start", 5));
      if (query_start_qs != Qnil) {
        query_start = atoi(RSTRING_PTR(query_start_qs));
      }

      // Get the query end.
      query_end = 0;
      VALUE query_end_qs = rb_hash_aref(query_string, rb_str_new("end", 3));
      if (query_end_qs != Qnil) {
        query_end = atoi(RSTRING_PTR(query_end_qs));
      }

      // Get the past N seconds of data.
      // @TODO they query method fails if we query for data newer than the last entry.
      VALUE query_recent = rb_hash_aref(query_string, rb_str_new("recent", 6));
      if (query_recent != Qnil) {
        query_end = (long int)time(NULL);
        long int history = atoi(RSTRING_PTR(query_recent));
        query_start = query_end - history;
      }

      // Create a new Statsrb object to query from.
      // @todo we probably need to assign a new array to @data to avoid messing up the pointers.
      VALUE tmp = rb_obj_dup(self);
      VALUE tmp_data = rb_ary_new();
      rb_iv_set(tmp, "@data", tmp_data);
      statsrb_query(tmp, rb_str_plus(rb_iv_get(self, "@split_file_dir"), statsrb_ns), statsrb_ns, INT2NUM(query_limit), INT2NUM(query_start), INT2NUM(query_end));
      statsrb_sort(tmp);

      int i, data_length = RARRAY_LEN(tmp_data);

      for (i = 0; i < data_length; i++) {
        rb_ary_push(body, rb_str_new("[", 1));
        rb_ary_push(body, rb_obj_as_string(rb_hash_aref(rb_ary_entry(tmp_data, i), statsrb_key_ts )));
        rb_ary_push(body, rb_str_new(",", 1));
        rb_ary_push(body, rb_obj_as_string(rb_hash_aref(rb_ary_entry(tmp_data, i), statsrb_key_v )));
        rb_ary_push(body, rb_str_new("]", 1));
        if (i < data_length - 1) {
          rb_ary_push(body, rb_str_new(",", 1));
        }
        rb_ary_push(body, rb_str_new("\n", 1));
      } 
      rb_ary_resize(tmp_data, 0);
    }
    rb_ary_push(body, rb_str_new("]}", 2));
    if (jsoncallback != Qnil) {
      rb_ary_push(body, rb_str_new(")", 1));
    }
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
static VALUE statsrb_constructor(VALUE self) {
  VALUE statsrb_data = rb_ary_new();
  rb_iv_set(self, "@data", statsrb_data);
  VALUE statsrb_split_file_dir = rb_str_new("/tmp", 4);
  rb_iv_set(self, "@split_file_dir", statsrb_split_file_dir);

  // Internal symbols for :ts, :ns and :v.
  VALUE statsrb_key_ts = rb_str_intern(rb_str_new2("ts"));
  rb_iv_set(self, "@key_ts", statsrb_key_ts);
  VALUE statsrb_key_ns = rb_str_intern(rb_str_new2("ns"));
  rb_iv_set(self, "@key_ns", statsrb_key_ns);
  VALUE statsrb_key_v = rb_str_intern(rb_str_new2("v"));
  rb_iv_set(self, "@key_v", statsrb_key_v);

  return self;
}

/* ruby calls this to load the extension */
void Init_statsrb(void) {
  VALUE klass = rb_define_class("Statsrb", rb_cObject);

  rb_define_method(klass, "initialize", statsrb_constructor, 0);
  rb_define_method(klass, "query", statsrb_query, 5);
  rb_define_method(klass, "sort", statsrb_sort, 0);
  rb_define_method(klass, "write", statsrb_write, 2);
  rb_define_method(klass, "split_write", statsrb_split_write, 2);
  rb_define_method(klass, "call", statsrb_rack_call, 1);
  // Define :attr_accessor (read/write instance var)
  // Note that this must correspond with a call to rb_iv_self() and it's string name must be @data.
  rb_define_attr(klass, "data", 1, 1);
  rb_define_attr(klass, "split_file_dir", 1, 1);
}
