#!/usr/bin/env ruby

if ARGV.empty?
  $stderr.puts "USAGE: #{$0} [benchmark_output]"
  exit 2
end

# parse the benchmark output
benchmark_results = File.readlines(ARGV[0])
  .grep(/.*benchmark-algorithm.*/)
  .map(&:chomp)
  .map do |line|
    tokens = line.split
    if tokens.size == 4
      [
        tokens[1],
        tokens[3].gsub('s','').to_f
      ]
    else
      nil
    end
  end.compact

# collect the algorithms' execution times into a Hash
benchmark_hash = Hash.new
benchmark_results.each do |result|
  name = result[0]
    .gsub(/benchmark-algorithm-/, '')
    .gsub(/-/, '::')
  unless benchmark_hash.has_key? name
    benchmark_hash[name] = Array.new
  end
  benchmark_hash[name] << result[1]
end

# print the table
def row(arr)
  puts '| ' + arr.join(' | ') + ' |'
end
puts "### Benchmarks\n\n"
row ['Algorithm', 'Average Time (s)']
row ['---', '---']
benchmark_hash.each do |name,times|
  n      = times.size
  ave    = times.sum / n
  stddev = Math.sqrt( 1.0 / n * times.map{ |t| (t-ave)**2 }.sum )
  err    = stddev / Math.sqrt(n)
  prec   = 2
  row ["`#{name}`", "$#{ave.round prec} \\pm #{err.round prec}$"]
end
puts ''
