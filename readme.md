statsrb
=======
This gem is a lightweight, self-contained (yet extendable) time series stats aggregation server written in C. Your data can easily be stored and retrieved via a simple REST API. Each data segment that you want to track is given a namespace that can be watched over time. For example, you could send a count of logged-in users every minute and track that metric over a timespan.

The gem is Rack compatable, meaning that it will function well with any Rack server. All REST queries are served with JSON data, optionally using jsonp if you need to execute cross-domain queries. Since this is fundamentally for time-series data, all requests are ordered chronologically.

If you do not need the REST API, the gem provides a Statsrb Ruby object that can be used to store data locally, also providing search and sort functionality.

The philosophy behind statsrb is that you can leverage the speed of a gem written in C while making it easy to extend using ruby. The internal data is stored using C structures and pointers to avoid any unnecessary memory allocation. This keeps things small and fast. Since time series data is simple, and quickly sorted, flat file storage is perfectly adequate. Without complicated databases, storage is kept to a minimum and the data can easily be used in other applications. Statsrb works great out of the box with a Rack server like Puma and a Javascript library like Flot. It can just as easily be integrated into any Ruby app to track without making HTTP requests.

As evidenced by the version number, this project is just getting started. Please feel free to submit issues or pull requests... any help would be appreciated :)

Current Version 0.2.0

Installation
------------
```
$ gem install statsrb
$ vim config.ru
$ mkdir /tmp/statsrb
$ rackup config.ru
```

Example config.ru
-----------------
```ruby
require 'statsrb'

s = Statsrb.new
# Writes statistics to files in this directory
# Make sure the directory exists and is writable.
s.split_file_dir = "/tmp/statsrb/"
# Flush @data to file when there are more than 9 values.
s.flush_count = 9
run s
```

REST URI Examples
-----------------
Save a statistic (flushes to a file based on the values of @split_file_dir and @flush_count)
```
http://localhost[:port]/put?name=test&value=123
http://localhost[:port]/put?name=test&value=123&time=123456789
```
Get a statistic within a time range
```
http://localhost[:port]/get/test?start=123456789&end=123456789
http://localhost[:port]/get/test?start=123456789&end=123456789&limit=1000
```
Get a statistic from a recent time
```
http://localhost[:port]/get/test?recent=86400&limit=100
```
Get a statistic to apply to a jsonp callback
```
http://localhost[:port]/get/test?recent=86400&limit=100&jsoncallback=mycallback
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
s.push (Time.now.to_i - 100), "test1", 36
s.push (Time.now.to_i - 50), "test2", 35

# Get filtered data based on namespace, limit, start and end timestamps.
# Statsrb::get() added in 0.1.4
pp s.get "none", 100, 0, 0
pp s.get "test1", 100, 0, 0
pp s.get "test2", 100, 0, 0

# Save the data to a single file.
s.write "/tmp/test.statsrb", "w+"

# Save the data to a separate files for each namespace.
s.split_write "/tmp/", "w+"

# Clear out the internal memory.
s.clear

# Query based on namespace, limit, start and end timestamps.
# Statsrb::query() deprecated in favor of Statsrb::read() in 0.1.4
s.query "/tmp/test.statsrb", "test1", 100, 0, 0  # 0.1.3
s.read "/tmp/test.statsrb", "test1", 100, 0, 0   # 0.1.4+

# Load a bunch of test data
s.load_test "test3", 1000

# Sort the data
s.sort

# Save the indexed data if you like.
s.write "/tmp/test.statsrb", "w+"
```

HTML/Javascript Example using jQuery and Flot
---------------------------------------------
(These files are located in /examples/)

First, ake sure you have pushed a bunch of data to the "test" namespace.
```
http://localhost[:port]/put?name=test&value=123
```

Serve the following index.html file from a different web server. Make sure to include the javascript files on that same server.
```html
<!DOCTYPE html>
<html>
<head>
  <title>Statsrb Javascript Example</title>
  <script language="Javascript" type="text/javascript" src="http://localhost:3000/js/jquery.min.js"></script> 
  <script language="Javascript" type="text/javascript" src="http://localhost:3000/js/jquery.flot.min.js"></script> 
  <script language="Javascript" type="text/javascript" src="http://localhost:3000/js/jquery.flot.time.min.js"></script> 
  <script language="Javascript" type="text/javascript" src="http://localhost:3000/js/statsrb.js"></script>

  <style type="text/css">
    body {background-color: #333; font-family: "Helvetica"; color: #f1f1f1;}
    #stats {width: 100%; height: 300px;}
  </style>
</head>

<body>
  <div id="stats"></div>
</body>

</html>
```

serve the following Javascript file with index.html in /js/statsrb.js
```javascript
/**
 * @file
 * This javascript obtains metrics from Statsrb and displays them using a simple
 * Flot time series graph.
 */

$(document).ready(function (context) {

  // Your data namespace to query, statsrb rack server domain name and port.
  var namespace = 'test';
  var domain = 'localhost';
  var port = '9292';

  // Plot options.
  var options = {
    xaxis: {
      mode: "time",
      timeformat: "%m/%d %I:%M"
    },
  }

  // Get the data from the server and plot it.
  $.ajax({type: 'GET',
    url: 'http://' + domain + ':' + port + '/get/' + namespace,
    crossDomain: true,
    dataType: 'jsonp',
    jsonp: 'jsoncallback',
    success: function(data) {
      $.each(data[namespace], function(key, value) {
        data[namespace][key][0] *= 1000;
      });

      $('#stats').before('<h1>Stats for: &quot;' + namespace + '&quot;</h1>');
      $.plot('#stats', [data[namespace]], options);
    }
  });

});
```

License
-------
Copyright 2013 Kevin Hankens

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
