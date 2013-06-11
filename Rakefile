require 'rake/testtask'

Rake::TestTask.new do |t|
  t.libs << 'test'
end

desc "Run tests"
task :default => :test

task :docs do
  puts `yardoc ext/statsrb/statsrb.c lib/statsrb.rb`
end
