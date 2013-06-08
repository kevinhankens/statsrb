require 'statsrb'

s = Statsrb.new
# Make sure this directory exists and is writable.
s.split_file_dir = "/tmp/statsr/"
s.flush_count = 5
run s
