require 'statsrb'

s = Statsrb.new
# Make sure this directory exists and is writable.
s.split_file_dir = "/tmp/statsr/"
run s
