#!/usr/bin/env ruby

require 'yaml'
require 'optparse'
require 'ostruct'
require 'fileutils'

require_relative 'src/Bindings_c.rb'

# default options
@args = OpenStruct.new
@args.action_yaml = ''
@args.out_dir     = ''
@args.out_prefix  = ''
@args.verbose     = false
@args.quiet       = false

# parse arguments
OptionParser.new do |o|
  o.banner = "USAGE: #{$0} [OPTIONS]..."
  o.separator ''
  o.separator 'OPTIONS:'
  o.on('-i', '--input [ACTION_YAML]', 'action function specification YAML') {|a| @args.action_yaml = a}
  o.separator ''
  o.on('-o', '--output [OUTPUT_DIR]', 'output directory', 'default: a subdirectory of ./chameleon-tree') {|a| @args.out_dir = a}
  o.on('-p', '--prefix [PREFIX]', 'if specified, each generated filename will start with [PREFIX]') {|a| @args.out_prefix = a}
  o.separator ''
  o.on('-v', '--verbose', 'print more output') {|a| @args.verbose = true}
  o.on('-q', '--quiet', 'print no output (unless --verbose)') {|a| @args.quiet = true}
  o.on_tail('-h', '--help', 'show this message') do
    puts o
    exit
  end
end.parse! ARGV
VERBOSE = @args.verbose
ACTION_YAML = @args.action_yaml

# start main Chameleon instance
@main = Chameleon.new
@main.verbose "OPTIONS: #{@args}"

# open action function specification
@main.error "option '--input' needs a value" if ACTION_YAML.empty?
@main.verbose "parsing #{ACTION_YAML}"
@main.error "input file '#{ACTION_YAML}' does not exist" unless File.exists? ACTION_YAML
main_spec = YAML.load_file ACTION_YAML

# parse algorithm info
algo_name = @main.get_spec main_spec, 'algorithm', 'name'
@main.verbose "algorithm name: #{algo_name}"

# make output files
def out_name(name)
  [ @args.out_prefix, name ].reject(&:empty?).join '_'
end
algo_dir = File.dirname ACTION_YAML
@main.error "directory '#{algo_dir}' does not exist" unless Dir.exists? algo_dir
out_dir = @args.out_dir.empty? ? File.join('chameleon-tree', algo_dir) : @args.out_dir
FileUtils.mkdir_p out_dir, verbose: @args.verbose
out_files = {
  :c_bindings => Bindings_c.new(File.join(out_dir, out_name('bind.cc')), algo_name),
}

# parse action functions
@main.get_spec(main_spec, 'actions').each do |action_spec|
  out_files[:c_bindings].bind action_spec
end

# print list of generated files
unless @args.quiet
  puts "Generated files:"
  out_files.each do |key,file|
    puts "  #{key} => #{file.out_name}"
  end
end

# close all generated files
out_files.values.each &:close
