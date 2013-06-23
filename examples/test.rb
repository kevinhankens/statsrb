#!/usr/bin/env ruby

require 'statsrb'
require 'json'
require 'pp'

# basic test
s = Statsrb.new
#s.push 34567, "kevin", 345
#s.push 12345, "kevin", 123
#s.push 23456, "kevin", 234
#s.push 45678, "melissa", 456
#s.push 56789, "melissa", 567
#s.push 67899, "melissa", 678


# test load and sort
s.load_test "kevin", 200000
s.load_test "melissa", 200000
s.load_test "benjamin", 200000
s.sort
#s.write "/tmp/statsrb/kevintest.log", "w+"
10.times do |i|
  pp s.get "kevin", 10, 0, 0
  pp s.get "melissa", 10, 0, 0
end

#t = Statsrb.new
#t.read "/tmp/statsrb/kevintest.log", "benjamin", 100, 107445, 0
#t.write "/tmp/statsrb/bentest.log", "w+"

# todo, call

#s.sort
#a = s.get "kevin", 100, 0, 0
#b = s.get "kevin", 1000, 0, 0
#c = s.get "kevin", 1000, 0, 0
#d = s.get "kevin", 1000, 0, 0
#e = s.get "kevin", 1000, 0, 0
#f = s.get "kevin", 1000, 0, 0
#g = s.get "kevin", 1000, 0, 0
#h = s.get "kevin", 10, 0, 0
# 100.times do |i|
#  s.load_test "kevin", 100
#  s.load_test "melissa", 100
#  s.load_test "benjamin", 100
#  s.get "benjamin", 10, 0, 0
# end
#pp h[0]
# s.write "/tmp/kevin.txt", "w+"
# s.split_write "/tmp/", "w+"


## test rack
#t = Statsrb.new
#t.read "/tmp/kevin", "blah", 10, 0, 0
#pp t.get "blah", 10, 0, 120
#
#env = {
#  "PATH_INFO" => "/PUT",
#  "QUERY_STRING" => "name=kevin&value=13"
#}
#
#11.times do |i|
#  pp t.call(env);
#end

## test big
# s.load_test 100000
# a = s.get "kevin", 100, 0, 0
# puts a.length
# pp a
