#!/usr/bin/env ruby

time_start = Time.now.to_f

require 'statsr'
require 'pp'

time_require = Time.now.to_f

puts "GEM"

s = Statsr.query "/var/log/statstest.log", ARGV[0], ARGV[1].to_f
puts s.length
pp s[0]
pp s[1]
pp s[2]
pp s[s.length - 3]
pp s[s.length - 2]
pp s[s.length - 1]

time_gem = Time.now.to_f

puts "TICKS"

ss = []
`fgrep #{ARGV[0]} /var/log/statstest.log | head -n#{ARGV[1]}`.split("\n").each do |line|
  l = line.split("\t")
  ss.push({:ts => l[0].to_i, :ns => l[1], :v => l[2].to_i})  
end

puts ss.length
pp ss[0]
pp ss[1]
pp ss[2]
pp ss[ss.length - 3]
pp ss[ss.length - 2]
pp ss[ss.length - 1]

time_tick = Time.now.to_f

puts "Require: #{time_require - time_start}"
puts "Query: #{time_gem - time_require}"
puts "Ticks: #{time_tick - time_gem}"
puts "Total: #{time_tick - time_start}"
