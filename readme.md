statsrb
=======
This gem is a lightweight self-contained, yet extendable stats aggregation server. The rack appliance receives time-series, integer stats via REST, and offers a simple API for searching and sorting. Each data segment is given a namespace that can be tracked through time. For example, your data for "users" would report an integer value with a timestamp that could then be queried to analyze changes over time.

All queries are served with JSON data, optionally using jsonp if you need to execute cross-domain queries. Since this is fundamentally for time-series data, all requests are ordered chronologically.

The philosophy behind statsrb is that you can leverage the speed of a gem written in C while making it easy to extend using ruby. It works great out of the box with a rack server and a Javascript library like Flot.

Installation
------------
```
$ gem install statsrb
$ vim config.ru
$ mkdir /tmp/statsr
$ rackup config.ru
```

Example config.ru
-----------------
```ruby
require 'statsrb'

s = Statsrb.new
# Make sure this directory exists and is writable.
s.split_file_dir = "/tmp/statsr/"
run s
```

REST URI Examples
-----------------
Save a statistic (for now it only writes to file on every 10th request)
```
http://localhost/put?name=test&value=123&time=123456789
```
Get a statistic within a time range
```
http://localhost/get/test?start=123456789&end=123456789&limit=100
```
Get a statistic from a recent time
```
http://localhost/get/test?recent=86400&limit=100
```
Get a statistic to apply to a jsonp callback
```
http://localhost/get/test?recent=123456789&limit=100&jsoncallback=mycallback
```

Ruby API Example
----------------
```ruby
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
```
