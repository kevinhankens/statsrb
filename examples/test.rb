#!/usr/bin/env ruby

require 'statsrb'
require 'json'
require 'pp'

s = Statsrb.new
#s.push 12345, "kevin", 123
#s.push 23456, "kevin", 234
#s.push 34567, "kevin", 345
#s.push 45678, "melissa", 456
#s.push 56789, "melissa", 567
#s.push 67899, "melissa", 678
s.load_test "kevin", 100000
s.load_test "melissa", 100000

pp s.get "kevin", 100, 0, 0

s.write "/tmp/kevin.txt", "w+"
s.split_write "/tmp/", "w+"

# s.load_test 100000
# a = s.get "kevin", 100, 0, 0
# puts a.length
# pp a
