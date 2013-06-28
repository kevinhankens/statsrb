#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Keeps track of a single event.
 */
typedef struct {
  int ns_index;
  int timestamp;
  int value;
} StatsrbEvent;

/**
 * Keeps track of available namespaces.
 */
typedef struct {
  char *namespace[256];
} StatsrbNS;

/**
 * Keeps track of internal storage.
 */
typedef struct {
  StatsrbEvent *event_list;
  int event_count;
  int event_memory;
  StatsrbNS *ns_list;
  int ns_count;
  int ns_memory;
} StatsrbInternal;

/**
 * Internal: retreives the internal storage.
 */
static StatsrbInternal* statsrb_get_internal(VALUE self) {
  StatsrbInternal *internal;
  Data_Get_Struct(self, StatsrbInternal, internal);

  return internal;
}

/**
 * Internal: allocates internal storage.
 */
static void statsrb_alloc_internal(VALUE self) {
  // Allocate internal memory for the StatsrbEvent structs.
  StatsrbEvent *eventlist = (StatsrbEvent *)calloc(1, sizeof(StatsrbEvent));

  // Allocate memory for the list of namespaces.
  StatsrbNS *nslist = (StatsrbNS *)calloc(1, sizeof(StatsrbNS));

  // Allocate memory for the pointer storage;
  StatsrbInternal *internalptr = (StatsrbInternal *)calloc(1, sizeof(StatsrbInternal));
  internalptr->event_list = eventlist;
  internalptr->event_count = 0;
  internalptr->event_memory = 0;
  internalptr->ns_list = nslist;
  internalptr->ns_count = 0;
  internalptr->ns_memory = 0;
  return Data_Wrap_Struct(self, 0, free, internalptr);
}

/**
 * Clears out the internal memory.
 *
 * @param VALUE self
 */
static void statsrb_data_clear_events(VALUE self) {
  StatsrbInternal *internal = statsrb_get_internal(self);

  // Allocate internal memory for the StatsrbEvent structs.
  StatsrbEvent *event_success = (StatsrbEvent *)realloc(internal->event_list, sizeof(StatsrbEvent));

  // Allocate memory for the list of namespaces.
  StatsrbNS *ns_success = (StatsrbNS *)realloc(internal->ns_list, sizeof(StatsrbNS));

  // Allocate memory for the pointer storage;
  if (event_success && ns_success) {
    StatsrbEvent *event_list = event_success;
    event_success = NULL;
    StatsrbNS *ns_list = ns_success;
    ns_success = NULL;
    internal->event_list = event_list;
    internal->event_count = 0;
    internal->event_memory = 0;
    internal->ns_list = ns_list;
    internal->ns_count = 0;
    internal->ns_memory = 0;
  }
  else {
    fprintf(stderr, "Error deallocating memory");
    return;
  }
}


/**
 * Returns the length of the internal storage.
 */
static VALUE statsrb_length(VALUE self) {
  StatsrbInternal *internal = statsrb_get_internal(self);
  if (!internal->event_count) {
    internal->event_count = 0;
  }

  return INT2NUM(internal->event_count);
}

/**
 * Debugging function.
 */
static void statsrb_debug_print_internal(VALUE self) {
  StatsrbInternal *internal = statsrb_get_internal(self);
  int i;

  //for (i = 0; i < internal->event_count; i++) {
    //fprintf(stdout, "Debug: ns: %s; ts: %d; v: %d\n", internal->ns_list[internal->event_list[i].ns_index].namespace, internal->event_list[i].timestamp, internal->event_list[i].value);
  //}
  fprintf(stdout, "Debug: count: %d memory: %d\n", internal->event_count, internal->event_memory);
}


/**
 * Implementation of quicksort algorithm.
 */
void time_sort(int left, int right, StatsrbEvent * event_list) {
  int i = left;
  int j = right;
  int p = (i + j) / 2;
  int pv = event_list[p].timestamp;
  StatsrbEvent * tmp = (StatsrbEvent *)malloc(sizeof(StatsrbEvent));

  while (i <= j) {
    while (event_list[i].timestamp < pv) {
      i++;
    }
    while (event_list[j].timestamp > pv) {
      j--;
    }
    if (i <= j) {
      memcpy(tmp, &event_list[i], sizeof(StatsrbEvent));
      memcpy(&event_list[i], &event_list[j], sizeof(StatsrbEvent));
      memcpy(&event_list[j], tmp, sizeof(StatsrbEvent));
      i++;
      j--;
    }
  }

  free(tmp);

  if (left < j) {
    time_sort(left, j, event_list);
  }
  if (i < right) {
    time_sort(i, right, event_list);
  }
}

/**
 * Sorts @data using a quicksort algorithm based on the hash element's timestamp.
 * @return [Hash] The sorted data
 */
static VALUE statsrb_sort(VALUE self) {
  StatsrbInternal *internal = statsrb_get_internal(self);
  if (internal->event_count > 0) {
    time_sort(0, internal->event_count - 1, internal->event_list);
  }
// @TODO what to return??
  return self;
}

/**
 * Internal: pushes a namespace onto the internal storage or retrieves a
 * preexisting one.
 *
 * @param VALUE self
 * @param const char *namespace
 *
 * @return Integer
 *   The pointer index of the namespace in @nslist.
 */
static int statsrb_data_push_ns(VALUE self, const char *namespace) {
  int i;
  StatsrbInternal *internal = statsrb_get_internal(self);

  for (i = 0; i < internal->ns_count; i++) {
    if (strcmp(internal->ns_list[i].namespace, namespace) == 0) {
      return i;
    }
  }

  int memory = (internal->ns_count + 1) * sizeof(StatsrbNS);
  StatsrbNS *success = (StatsrbNS *)realloc(internal->ns_list, memory);

  if (success) {
    internal->ns_list = success;
    success = NULL;
    strcpy(internal->ns_list[internal->ns_count].namespace, namespace);
    internal->ns_count++;
    return internal->ns_count - 1;
  }
  else {
    fprintf(stderr, "Error allocating memory");
    return;
  }

}

/**
 * Internal: pushes a data event onto the internal storage.
 *
 * @param VALUE self
 * @param const char *namespace
 * @param int timestamp
 * @param int value
 */
static void statsrb_data_push_event(VALUE self, const char *namespace, int timestamp, int value) {
  StatsrbInternal *internal = statsrb_get_internal(self);

  // Get the index of the namespace pointer.
  int ns_index = statsrb_data_push_ns(self, namespace);

  // If it appears that we are approaching the end of the memory block, allocate
  // some more.
  // @TODO 2x memory is a little nuts, maybe throttle this back a bit?
  if ((sizeof(StatsrbEvent) * internal->event_count) > (internal->event_memory * .9)) {
    internal->event_memory = (2* internal->event_count) * sizeof(StatsrbEvent);
    StatsrbEvent *success = (StatsrbEvent *)realloc(internal->event_list, internal->event_memory);
    if (success) {
      internal->event_list = success;
      success = NULL;
    }
    else {
      fprintf(stderr, "Error allocating memory");
      return;
    }
  }

  // Set the values;
  internal->event_list[internal->event_count].timestamp = timestamp;
  internal->event_list[internal->event_count].ns_index = ns_index;
  internal->event_list[internal->event_count].value = value;

  // Track the count by saving the new pointer.
  internal->event_count++;
}

/**
 * Creates a ruby hash from event VALUEs.
 *
 * @param VALUE self
 * @param VALUE ts
 * @param VALUE ns
 * @param VALUE v
 */
VALUE statsrb_create_rb_event_hash(VALUE self, VALUE ts, VALUE ns, VALUE v) {
  // @data hash key symbols.
  VALUE statsrb_key_ts = rb_iv_get(self, "@key_ts");
  VALUE statsrb_key_ns = rb_iv_get(self, "@key_ns");
  VALUE statsrb_key_v = rb_iv_get(self, "@key_v");

  VALUE statsrb_event = rb_hash_new();
  rb_hash_aset(statsrb_event, statsrb_key_ts, ts);
  rb_hash_aset(statsrb_event, statsrb_key_ns, ns);
  rb_hash_aset(statsrb_event, statsrb_key_v, v);

  return statsrb_event;
}

/**
 * Pushes a stat onto the statsrb object.
 * @param timestamp [Number]
 * @param namespace [String]
 * @param value [Number]
 * @return [Statsrb] A reference to the object.
 */
static VALUE statsrb_push(VALUE self, VALUE timestamp, VALUE namespace, VALUE value) {
  int ts = NUM2INT(timestamp);
  int v = NUM2INT(value);
  const char *ns = RSTRING_PTR(namespace);
  statsrb_data_push_event(self, ns, ts, v);
  return self;
}

/**
 * Retrieves internal data based on specified filters.
 * @param namespace [String]
 * @param limit [Number]
 * @param start_time [Number]
 * @param end_time [Number]
 * @return [Array] An array of data hashes.
 */
static void statsrb_get(VALUE self, VALUE query_ns, VALUE query_limit, VALUE query_start, VALUE query_end) {
  // @TODO maybe it would be sane to make a new statsrb object and then just have
  // methods to dump everything to ary, json, etc.
  StatsrbInternal *internal = statsrb_get_internal(self);
  int tmp_ts, tmp_v, tmp_i;

  VALUE filtered_data = rb_ary_new();
  VALUE rb_ns_list = rb_ary_new();
  VALUE statsrb_event;

  int i = 0;
  int filtered_count = 0;

  int limit = NUM2INT(query_limit);
  int qstart = NUM2INT(query_start);
  int qend = NUM2INT(query_end);

  VALUE rb_ns;

  // Create rb strings for the namespaces.
  signed int found = -1;
  for (i = 0; i < internal->ns_count; i++) {
    rb_hash_aset(rb_ns_list, INT2NUM(i), rb_str_new2(internal->ns_list[i].namespace));
    if (strcmp(RSTRING_PTR(query_ns), RSTRING_PTR(rb_hash_aref(rb_ns_list, INT2NUM(i)))) == 0) {
      memcpy(&found, &i, sizeof(int));
    }
  }

  // Return right away if the namespace doesn't exist.
  if (found == -1) {
    rb_ary_resize(filtered_data, (long) 0);
    return filtered_data;
  }

  // Iterate through the in-memory data to find matches.
  for (i = 0; i < internal->event_count; i++) {
    if (found == internal->event_list[i].ns_index
        && (qstart == 0 || internal->event_list[i].timestamp >= qstart)
        && (qend == 0 || internal->event_list[i].timestamp <= qend)) {

      memcpy(&tmp_ts, &internal->event_list[i].timestamp, sizeof(int));
      memcpy(&tmp_v, &internal->event_list[i].value, sizeof(int));

      statsrb_event = statsrb_create_rb_event_hash(
        self,
        INT2NUM(tmp_ts),
        rb_hash_aref(rb_ns_list, INT2NUM(found)),
        INT2NUM(tmp_v)
      );

      rb_ary_push(filtered_data, statsrb_event);
      filtered_count++;
    }

    if (limit > 0 && filtered_count == limit) {
      break;
    }
  }

  rb_ary_resize(filtered_data, filtered_count);
  return filtered_data;
}

/**
 * Locates data from a specified file and loads into @data.
 * @param filepath [String]
 * @param namespace [String]
 * @param limit [Number]
 * @param start_time [Number]
 * @param end_time [Number]
 * @return [Statsrb] A reference to the object.
 */
static VALUE statsrb_read(VALUE self, VALUE logfile, VALUE query_ns, VALUE query_limit, VALUE query_start, VALUE query_end) {
  FILE * file;
  int line_size = 512;
  char *line = (char *) malloc(line_size);
  char *tmp_ns = (char *) malloc(256);
  const char *filepath = RSTRING_PTR(logfile);
  const char *query_ns_char = RSTRING_PTR(query_ns);
  int tmp_v, tmp_ts;

  // Convert into an int that ruby understands.
  int limit = NUM2INT(query_limit);
  int qstart = NUM2INT(query_start);
  int qend = NUM2INT(query_end);

  file = fopen(filepath, "r");
  if (file == NULL) {
    fprintf(stderr, "File error: could not open file %s for reading.", filepath);
    return self;
  }

  int count = 0;

  while (NULL != fgets(line, line_size, file) && count < limit) {
    // strstr doesn't work with newline chars.
    size_t len = strlen(line) - 1;
    if (line[len] == '\n');
        line[len] = '\0';

    // If the namespace is in the row, explode it.
    if (line[0] != '\0' && line[0] != '\n' && strchr(line, query_ns_char[0]) && strstr(line, query_ns_char)) {
      //VALUE statsrb_event = rb_hash_new();

      // I tried sscanf for convenience, but it was predictably slower.
      //int statsrb_ts, statsrb_v;
      //sscanf(line, "%d\t%*s\t%d", &statsrb_ts, &statsrb_v);

      // @TODO this should something more robust than atoi.
      tmp_ts = atoi(strtok(line, "\t"));

      if (tmp_ts != NULL && (qstart == 0 || tmp_ts >= qstart) && (qend == 0 || tmp_ts <= qend)) {
        // @TODO this should probably use the actual namespace if we do wildcard queries.
        strcpy(tmp_ns, strtok(NULL, "\t"));
        //strtok(NULL, "\t");
        tmp_v = atoi(strtok(NULL, "\0"));

        // @TODO this should really query the namespace exactly instead of just relying on strstr.
        //if (rb_str_cmp(query_ns, statsrb_str_empty) == 0 || rb_str_cmp(query_ns, statsrb_str_ns) == 0) {
        if (tmp_ts && (tmp_v || tmp_v == 0)) {
          statsrb_data_push_event(self,
            tmp_ns,
            tmp_ts,
            tmp_v);
          count++;
        }
      }
    }
  }

  // terminate
  fclose (file);
  free (line);
  free (tmp_ns);

  return self;
}


/**
 * Writes the @data in memory to a specified file.
 * @param filepath [String]
 * @param filemode [String]
 * @return [Statsrb] A reference to the object.
 */
static VALUE statsrb_write(VALUE self, VALUE logfile, VALUE mode) {
  FILE * file;
  const char *filepath = RSTRING_PTR(logfile);
  const char *filemode = RSTRING_PTR(mode);

  StatsrbInternal *internal = statsrb_get_internal(self);
  int i;

  file = fopen(filepath, filemode);
  if (file==NULL) {
    fprintf(stderr, "File error: could not open file %s mode %s.", filepath, filemode);
    return self;
  }

  // Iterate through the data array, writing the data as we go.
  for (i = 0; i < internal->event_count; i++) {
    // @TODO make sure that these values are not empty before writing.
    fprintf(file,
            "%d\t%s\t%d\n",
            internal->event_list[i].timestamp,
            internal->ns_list[internal->event_list[i].ns_index].namespace,
            internal->event_list[i].value
    );
  }

  fclose (file);
  return self;
}

/**
 * Locates data from a specified file and loads into @data.
 * @param filepath [String]
 * @param namespace [String]
 * @param limit [Number]
 * @param start_time [Number]
 * @param end_time [Number]
 * @return [Statsrb] A reference to the object.
*/
static VALUE statsrb_split_write(VALUE self, VALUE logdir, VALUE mode) {
  StatsrbInternal *internal = statsrb_get_internal(self);
  int i, ii, ns_len;

  VALUE filename;
  VALUE klass = rb_obj_class(self);
  VALUE tmp = rb_class_new_instance(0, NULL, klass);

  for (i = 0; i < internal->ns_count; i++) {
    for (ii = 0; ii < internal->event_count; ii++) {
      if (strcmp(internal->ns_list[i].namespace, internal->ns_list[internal->event_list[ii].ns_index].namespace) == 0) {
        statsrb_data_push_event(tmp,
          internal->ns_list[internal->event_list[ii].ns_index].namespace,
          internal->event_list[ii].timestamp,
          internal->event_list[ii].value);
      }
    }

    // If there is no trailing slash on the log dir, add one.
    const char *filepath = RSTRING_PTR(logdir);
    size_t len = strlen(filepath);
    if (filepath[len - 1] != '/') {
      logdir = rb_str_plus(logdir, rb_str_new2("/"));
    }
    filename = rb_str_new2(internal->ns_list[i].namespace);
    statsrb_write(tmp, rb_str_plus(logdir, filename), mode);
    statsrb_data_clear_events(tmp);
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
 * Returns a rack-compatible response.
 * @param env [Hash]
*/
static VALUE statsrb_rack_call(VALUE self, VALUE env) {
  VALUE response = rb_ary_new();
  VALUE headers = rb_hash_new();
  VALUE body = rb_ary_new();

  char *path = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("PATH_INFO")));

  rb_hash_aset(headers, rb_str_new2("Content-Type"), rb_str_new2("text/json"));

  // Parse the query string
  char *qs = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("QUERY_STRING")));
  VALUE query_string = statsrb_parse_qs(qs);

  //const char *method = RSTRING_PTR(rb_hash_aref(env, rb_str_new2("REQUEST_METHOD")));
  // @TODO consider moving the request method to the proper REQUEST_METHOD
  const char *method_get = "get";
  const char *method_getu = "GET";
  const char *method_put = "put";
  const char *method_putu = "PUT";
  // Remove the leading slash.
  path++;
  const char *method = strtok(path, "/\0");
  if (method && (strcmp(method, method_put) == 0 || strcmp(method, method_putu) == 0)) {
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

      statsrb_data_push_event(self, RSTRING_PTR(statsrb_ns), statsrb_ts, statsrb_v);

      int data_length = NUM2INT(statsrb_length(self));

      rb_ary_push(body, rb_obj_as_string(INT2NUM(data_length)));

      if (data_length >= NUM2INT(rb_iv_get(self, "@flush_count"))) {
        statsrb_sort(self);
        statsrb_split_write(self, rb_iv_get(self, "@split_file_dir"), rb_str_new2("a+"));
        statsrb_data_clear_events(self);
      }

      rb_ary_push(body, statsrb_ns);
    }
  }
  else if (method && (strcmp(method, method_get) == 0 || strcmp(method, method_getu) == 0)) {
    const char * statsrb_str_ns = strtok(NULL, "/\0");
    if (statsrb_str_ns == NULL) {
      statsrb_str_ns = "data";
    }

    VALUE jsoncallback = rb_hash_aref(query_string, rb_str_new("jsoncallback", 12));
    if (jsoncallback != Qnil) {
      rb_ary_push(body, rb_str_plus(jsoncallback, rb_str_new("(", 1)));
    }
    // @TODO move this to a to_json method.
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
      // @TODO the query method fails if we query for data newer than the last entry.
      VALUE query_recent = rb_hash_aref(query_string, rb_str_new("recent", 6));
      if (query_recent != Qnil) {
        query_end = (long int)time(NULL);
        long int history = atoi(RSTRING_PTR(query_recent));
        query_start = query_end - history;
      }

      // Create a new Statsrb object to query from.
      VALUE klass = rb_obj_class(self);
      VALUE tmp = rb_class_new_instance(0, NULL, klass);

      statsrb_read(tmp, rb_str_plus(rb_iv_get(self, "@split_file_dir"), statsrb_ns), statsrb_ns, INT2NUM(query_limit), INT2NUM(query_start), INT2NUM(query_end));
      statsrb_sort(tmp);

      int i, data_length = NUM2INT(statsrb_length(tmp));
      StatsrbInternal *internal = statsrb_get_internal(tmp);

      for (i = 0; i < data_length; i++) {
        rb_ary_push(body, rb_str_new("[", 1));
        rb_ary_push(body, rb_obj_as_string(INT2NUM(internal->event_list[i].timestamp)));
        rb_ary_push(body, rb_str_new(",\"", 2));
        rb_ary_push(body, rb_str_new2(internal->ns_list[internal->event_list[i].ns_index].namespace));
        rb_ary_push(body, rb_str_new("\",", 2));
        rb_ary_push(body, rb_obj_as_string(INT2NUM(internal->event_list[i].value)));
        rb_ary_push(body, rb_str_new("]", 1));

        if (i < data_length - 1) {
          rb_ary_push(body, rb_str_new(",", 1));
        }
        rb_ary_push(body, rb_str_new("\n", 1));
      }
      statsrb_data_clear_events(tmp);
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
 * Populates the internal storage with test data.
 */
static void statsrb_load_test(VALUE self, VALUE ns, VALUE amt) {
  StatsrbInternal *internal = statsrb_get_internal(self);
  int i, val;
  srand(time(NULL));
  for (i = 0; i < NUM2INT(amt); i++) {
    val = rand();
    statsrb_data_push_event(self, RSTRING_PTR(ns), val + 100, val + 1);
  }
  statsrb_debug_print_internal(self);
  fprintf(stdout, "Debug: count: %d\n", internal->event_count);
}

/**
 * Class constructor, sets up an instance variable.
 */
static VALUE statsrb_constructor(VALUE self) {
  VALUE statsrb_data = rb_ary_new();
  rb_iv_set(self, "@data", statsrb_data);
  VALUE statsrb_split_file_dir = rb_str_new("/tmp", 4);
  rb_iv_set(self, "@split_file_dir", statsrb_split_file_dir);
  rb_iv_set(self, "@flush_count", INT2NUM(9));

  // Internal symbols for :ts, :ns and :v.
  VALUE statsrb_key_ts = rb_str_intern(rb_str_new2("ts"));
  rb_iv_set(self, "@key_ts", statsrb_key_ts);
  VALUE statsrb_key_ns = rb_str_intern(rb_str_new2("ns"));
  rb_iv_set(self, "@key_ns", statsrb_key_ns);
  VALUE statsrb_key_v = rb_str_intern(rb_str_new2("v"));
  rb_iv_set(self, "@key_v", statsrb_key_v);

  return self;
}

/**
 * Init the Statsrb class.
 */
void Init_statsrb(void) {
  // @author Kevin Hankens
  VALUE klass = rb_define_class("Statsrb", rb_cObject);

  // Instance methods and properties.
  rb_define_alloc_func(klass, statsrb_alloc_internal);
  rb_define_method(klass, "initialize", statsrb_constructor, 0);
  rb_define_method(klass, "query", statsrb_read, 5);
  rb_define_method(klass, "read", statsrb_read, 5);
  rb_define_method(klass, "get", statsrb_get, 4);
  rb_define_method(klass, "load_test", statsrb_load_test, 2);
  rb_define_method(klass, "length", statsrb_length, 0);
  rb_define_method(klass, "sort", statsrb_sort, 0);
  rb_define_method(klass, "write", statsrb_write, 2);
  rb_define_method(klass, "split_write", statsrb_split_write, 2);
  rb_define_method(klass, "push", statsrb_push, 3);
  rb_define_method(klass, "clear", statsrb_data_clear_events, 0);
  rb_define_method(klass, "call", statsrb_rack_call, 1);
  // Define :attr_accessor (read/write instance var)
  // Note that this must correspond with a call to rb_iv_self() and it's string name must be @data.
  // An array of hashes keyed with :ts(timestamp), :ns(namespace) and :v(value) e.g. [!{:ts => Time.now.to_i, :ns => "test", :v => 33}]
  rb_define_attr(klass, "data", 1, 1);
  // The file directory to write when splitting namespaces. @see #split_write
  rb_define_attr(klass, "split_file_dir", 1, 1);
  // When used with a rack server, the max count of @data before flushing and writing to file.
  rb_define_attr(klass, "flush_count", 1, 1);
}
