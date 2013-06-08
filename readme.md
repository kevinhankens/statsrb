statsrb
=======
This gem is a lightweight self-contained, yet extendable stats aggregation server. The rack appliance receives time-series, integer stats via REST, and offers a simple API for searching and sorting. Each data segment is given a namespace that can be tracked through time. For example, your data for "users" would report an integer value with a timestamp that could then be queried to analyze changes over time.

All queries are served with JSON data, optionally using jsonp if you need to execute cross-domain queries. Since this is fundamentally for time-series data, all requests are ordered chronologically.

The philosophy behind statsrb is that you can leverage the speed of a gem written in C while making it easy to extend using ruby. It works great out of the box with a rack server and a Javascript library like Flot.

Installation
------------
```
gem install statsrb
rackup config.ru
```

REST Examples
-------------
Save a statistic
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
```

