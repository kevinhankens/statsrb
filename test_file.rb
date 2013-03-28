#!/usr/bin/env ruby

require 'statsr'
require 'pp'

s = Statsr.new
10000.times do |t|
  h = {:ts => Time.now.to_i, :ns => "kevin", :v => t}
  s.data.push h
  h = {:ts => Time.now.to_i, :ns => "melissa", :v => t+1}
  s.data.push h
  h = {:ts => Time.now.to_i, :ns => "ben", :v => t+2}
  s.data.push h
end

puts "done creating"

s.splitwrite "/home/khankens/splittest/", "a+"
puts "done splitting"

#`cat /home/khankens/ksp.log`
