#!/usr/bin/env ruby

require 'statsr'
require 'pp'

s = Statsr.query ARGV[0], ARGV[1].to_i
puts s.length
pp s[0]
pp s[1]
pp s[2]
