#!/usr/bin/env ruby

require 'pry'

if ARGV.empty?
  $stderr.puts "USAGE: #{$0} [benchmark_outputs]"
  exit 2
end

results = Hash.new
suites = Array.new

ARGV.each do |benchmark_file|
  File.readlines(benchmark_file).grep(/.*benchmark-.*/).map(&:chomp).each do |line|

    tokens = line.split
    next unless tokens.size == 6

    name      = tokens[3]
    time      = tokens[5].gsub('s','').to_f
    suite     = name.split('-')[1]
    algo_name = name.split('-')[2..-1].join('::')

    results[algo_name] = Hash.new unless results.has_key? algo_name
    results[algo_name][suite] = time
    suites << suite
  end
end

suites.uniq!

def row(arr)
  puts '| ' + arr.join(' | ') + ' |'
end
puts """### Benchmarks
Algorithm execution times in seconds

"""
row ['Algorithm', *suites.map{|suite|suite.gsub /_/, ' '}]
row (suites.size+1).times.map{ |i| '---' }

results.each do |algo_name, algo_results|
  times = suites.map{ |suite| algo_results[suite] }
  row ["`#{algo_name}`", *times]
end
