require 'statsrb'

s = Statsrb.new
# Make sure this directory exists and is writable.
s.split_file_dir = "/tmp/statsrb/"
# Flush @data to file when there are more than 9 values.
s.flush_count = 9
run s
