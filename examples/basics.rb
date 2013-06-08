#!/usr/bin/env ruby

require 'statsrb'
require 'pp'

# Create new data with various timestamps, namespaces and values.
s = Statsrb.new
s.push Time.now.to_i, "test1", 33
s.push 123456789, "test1", 34
s.push (Time.now.to_i - 50), "test2", 35
s.push (Time.now.to_i - 100), "test1", 36

# Save the data to a single file.
s.write "/tmp/test.statsrb", "w+"

# Save the data to a separate files for each namespace.
s.split_write "/tmp/", "w+"

# Load data from a file
t = Statsrb.new

# Query based on namespace, limit, start and end timestamps.
t.query "/tmp/test.statsrb", "test1", 100, 0, 0

# Sort the data
t.sort

# Save the indexed data if you like.
t.write "/tmp/test.statsrb", "w+"

# See what you are working with.
pp t.data
