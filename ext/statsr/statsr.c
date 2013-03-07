#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static VALUE query(VALUE self, VALUE logfile, VALUE query_ns, int query_limit) {
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
  VALUE statsr_data = rb_ary_new();

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
      //VALUE statsr_str_ns = rb_str_new2(strtok(NULL, "\t"));
      strtok(NULL, "\t");
      int statsr_v = atoi(strtok(NULL, "\0"));

      // @TODO this should really query the namespace exactly instead of just relying on strstr.
      //if (rb_str_cmp(query_ns, statsr_str_empty) == 0 || rb_str_cmp(query_ns, statsr_str_ns) == 0) {
      if (statsr_ts && statsr_v) {
        rb_hash_aset(statsr_event, statsr_key_ts, INT2NUM(statsr_ts));
        //rb_hash_aset(statsr_event, statsr_key_ns, statsr_str_ns);
        rb_hash_aset(statsr_event, statsr_key_ns, query_ns);
        rb_hash_aset(statsr_event, statsr_key_v, INT2NUM(statsr_v));
        rb_ary_push(statsr_data, statsr_event);
        count++;
      }
    }
  }

  // terminate
  fclose (file);
  free (line);

  return statsr_data;
}

/* ruby calls this to load the extension */
void Init_statsr(void) {
  /* assume we haven't yet defined Hola */
  VALUE klass = rb_define_class("Statsr",
      rb_cObject);

  // Sets up the data
  rb_define_singleton_method(klass, "query", query, 3);
}
