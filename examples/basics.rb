#!/usr/bin/env ruby

require 'statsrb'
require 'pp'

# Create new data with various timestamps, namespaces and values.
s = Statsrb.new
s.data = [
  {:ts => Time.now.to_i,         :ns => "test1", :v => 33},
  {:ts => 123456789,             :ns => "test2", :v => 34},
  {:ts => (Time.now.to_i - 50),  :ns => "test1", :v => 35},
  {:ts => (Time.now.to_i - 100), :ns => "test1", :v => 36},
]

# Save the data to a single file.
s.write "/tmp/test.statsr", "w+"

# Save the data to a separate files for each namespace.
s.split_write "/tmp/", "w+"

# Load data from a file
t = Statsrb.new

# Query based on namespace, limit, start and end timestamps.
t.query "/tmp/test.statsr", "test1", 100, 0, 0

# Sort the data
t.sort

# Save the indexed data if you like.
t.write "/tmp/test.statsr", "w+"

# See what you are working with.
pp t.data
