#!/usr/bin/env ruby

require 'statsrb'
require 'json'
require 'pp'

s = Statsrb.new
s.load_test 100000
a = s.get "kevin", 100, 0, 0
puts a.length
pp a

b = s.get "kevin", 300, 0, 0
puts b.length
puts b.to_json
