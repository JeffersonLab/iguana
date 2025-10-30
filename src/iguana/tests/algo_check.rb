#!/usr/bin/env ruby
# runs various checks on algorithms, to make sure they have everything they should have

# list of algorithms, excluding base-class algorithm
src_dir   =  File.realpath "#{__dir__}/../../.."
algo_list =  Dir.glob("src/**/Algorithm.h").grep_v("src/iguana/algorithms/Algorithm.h")
algo_list += Dir.glob("src/**/AlgorithmSequence.h")

# check for certain docstrings
algo_list.each do |algo|
  puts "check docstrings in #{algo}"
  # check for `@algo_brief`
  system "grep -q '@algo_brief' #{algo}"
  raise "algorithm #{algo} lacks an '@algo_brief' docstring" unless $?.success?
  # check for `@algo_type`
  unless algo.match? /AlgorithmSequence.h/
    system "grep -q '@algo_type_' #{algo}"
    raise "algorithm #{algo} lacks an '@algo_type_*' docstring" unless $?.success?
  end
end
