#!/usr/bin/env ruby
# runs various checks on algorithms, to make sure they have everything they should have

# list of algorithms, excluding base-class algorithm
src_dir   =  File.realpath "#{__dir__}/../../.."
algo_list =  Dir.glob("src/**/Algorithm.h").grep_v("src/iguana/algorithms/Algorithm.h")
algo_list += Dir.glob("src/**/AlgorithmSequence.h")

# check for certain docstrings
algo_list.each do |algo_header|
  puts "check docstrings in #{algo_header}"
  # check for `@algo_brief`
  system "grep -q -w '@algo_brief' #{algo_header}"
  raise "algorithm header #{algo_header} lacks an '@algo_brief' docstring" unless $?.success?
  # check for `@algo_type`
  unless algo_header.match? /AlgorithmSequence.h/
    system "grep -q '@algo_type_' #{algo_header}"
    raise "algorithm header #{algo_header} lacks an '@algo_type_*' docstring" unless $?.success?
  end
  # check for `@doc_config`
  config_file = "#{File.dirname algo_header}/Config.yaml"
  if File.exist? config_file
    system "grep -q -w '@doc_config' #{algo_header}"
    raise "algorithm header #{algo_header} lacks a '@doc_config' docstring; this is needed since the algorithm has a 'Config.yaml' file" unless $?.success?
  end
end
