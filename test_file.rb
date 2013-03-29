#!/usr/bin/env ruby

require 'statsr'
require 'pp'

s = Statsr.new

1000.times do |t|
  h = {:ts => Time.now.to_i - t, :ns => "kevin", :v => 20 + Random.rand(11)}
  s.data.push h
  h = {:ts => Time.now.to_i - t, :ns => "melissa", :v => 30 + Random.rand(11)}
  s.data.push h
  h = {:ts => Time.now.to_i - t, :ns => "ben", :v => 40 + Random.rand(11)}
  s.data.push h
end
s.sort

puts "done creating"

s.split_write "/tmp/statsr/", "w+"
puts "done splitting"

# puts `cat /home/khankens/ksp.log`
