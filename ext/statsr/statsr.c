#include <ruby.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static VALUE query(VALUE self, VALUE query_ns, int query_limit) {
  FILE * file;
  //char line[256];
  int line_size = 256;
  char *line = (char *) malloc(line_size);

  VALUE statsr_key_ts = rb_str_new2("ts");
  VALUE statsr_key_ns = rb_str_new2("ns");
  VALUE statsr_key_v = rb_str_new2("v");
  VALUE statsr_str_empty = rb_str_new2("");

  int statsr_timestamp;
  int statsr_value;
  int limit = NUM2INT(query_limit);

  VALUE statsr_data = rb_ary_new();

  file = fopen ( "/var/log/statstest.log" , "rb" );
  if (file==NULL) {fputs ("File error",stderr); exit (1);}

  int count = 0;

  while (NULL != fgets(line, line_size, file) && count < limit) {
    if (line[0] != '\0' && line[0] != '\n') {
      VALUE statsr_event = rb_hash_new();
      VALUE statsr_str_ts = rb_str_new2(strtok(line, "\t"));
      VALUE statsr_str_ns = rb_str_new2(strtok(NULL, "\t"));
      VALUE statsr_str_v = rb_str_new2(strtok(NULL, "\n"));

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
  rb_define_singleton_method(klass, "query", query, 2);
}
