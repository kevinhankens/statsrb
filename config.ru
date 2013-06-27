require 'statsrb'

s = Statsrb.new
# Writes statistics to files in this directory
# Make sure the directory exists and is writable.
s.split_file_dir = "/tmp/statsrb/"
# Flush @data to file when there are more than 9 values.
s.flush_count = 9
run s

