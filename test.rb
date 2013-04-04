#!/usr/bin/env ruby

time_start = Time.now.to_f

require 'statsrb'
require 'pp'
require 'json'

time_require = Time.now.to_f

s = Statsrb.new
s.query "/tmp/statsr/kv3", "kv3", 10, 0, 0
s.sort
pp s.data


abort

puts "GEM"

s = Statsrb.new
s.query "/var/log/statstest.log", ARGV[0], ARGV[1].to_f, 1357997415, 1357997429
s.sort
s.write "/home/khankens/ktest.log", "w+"
h= {:ts => Time.now.to_i, :ns => "kevin", :v => 33}
s.data.push h
pp s.data
s.write "/home/khankens/ktest.log", "a"

abort

#puts s.data.to_json

pp s.data[0]
pp s.data[1]
pp s.data[2]
pp s.data[s.data.length - 3]
pp s.data[s.data.length - 2]
pp s.data[s.data.length - 1]

time_gem = Time.now.to_f

s.sort;

puts s.data.length
pp s.data[0]
pp s.data[1]
pp s.data[2]
pp s.data[s.data.length - 3]
pp s.data[s.data.length - 2]
pp s.data[s.data.length - 1]

time_sort = Time.now.to_f

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


total_time_sort = time_sort - time_gem
total_time_query = time_gem - time_require

puts "Require: #{time_require - time_start}"
puts "Query: #{total_time_query}"
puts "Sort: #{total_time_sort}"
puts "Q+S: #{total_time_query + total_time_sort}"
puts "Ticks: #{time_tick - time_sort}"
puts "Total: #{time_tick - time_start}"

