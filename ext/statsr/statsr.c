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

  file = fopen (filepath, "rb");
  if (file==NULL) {fputs ("File error",stderr); exit (1);}

  int count = 0;

  while (NULL != fgets(line, line_size, file) && count < limit) {

    size_t len = strlen(line) - 1;
 
    if (line[len] == '\n');
        line[len] = '\0';

    if (line[0] != '\0' && line[0] != '\n' && strstr(line, query_ns_char)) {
    //if (line[0] != '\0' && line[0] != '\n') {
      VALUE statsr_event = rb_hash_new();
      VALUE statsr_str_ts = rb_str_new2(strtok(line, "\t"));
      VALUE statsr_str_ns = rb_str_new2(strtok(NULL, "\t"));
      VALUE statsr_str_v = rb_str_new2(strtok(NULL, "\0"));

      if (rb_str_cmp(query_ns, statsr_str_empty) == 0 || rb_str_cmp(query_ns, statsr_str_ns) == 0) {
        rb_hash_aset(statsr_event, statsr_key_ts, statsr_str_ts);
        rb_hash_aset(statsr_event, statsr_key_ns, statsr_str_ns);
        rb_hash_aset(statsr_event, statsr_key_v, statsr_str_v);
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
