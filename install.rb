#!/usr/bin/env ruby

require 'optparse'
require 'fileutils'

SourceDir = __dir__

# parse options
options = {
  build:   "#{Dir.pwd}/build-iguana",
  install: "#{Dir.pwd}/iguana",
  hipo:    "#{Dir.pwd}/hipo",
  fmt:     nil,
  clean:   false,
  purge:   false,
}
parser = OptionParser.new
parser.program_name = $0
parser.separator ''
parser.separator 'OPTIONS:'
parser.separator ''
parser.on("-b", "--build [BUILD DIR]", "Directory for buildsystem", "Default: #{options[:build]}")
parser.separator ''
parser.on("-i", "--install [INSTALL DIR]", "Directory for installation", "Default: #{options[:install]}")
parser.separator ''
parser.on("--hipo [HIPO DIR]", "Path to HIPO installation", "Default: #{options[:hipo]}", "if not found, tries env var $HIPO")
parser.on("--fmt [FMT DIR]", "Path to fmt installation", "Default: assumes system installation" )
parser.separator 'TROUBLESHOOTING:'
parser.on("--clean", "Remove the buildsystem at [BUILD DIR] beforehand")                 
parser.on("--purge", "Remove the installation at [INSTALL DIR] beforehand")                 
parser.parse!(into: options)

# check for HIPO installation, or fallback to $HIPO
options[:hipo] = ENV['HIPO'] unless Dir.exists? options[:hipo]

# use realpaths for dependencies
[ :hipo, :fmt ].each do |dep|
  unless options[dep].nil?
    if Dir.exists? options[dep]
      options[dep] = File.realpath options[dep]
    else
      $stderr.puts "ERROR: directory '#{options[dep]}' for option '--#{dep.to_s}' does not exist"
      exit 1
    end
  end
end

# print the options
puts "SET OPTIONS:"
options.each do |k,v| puts "#{k.to_s.rjust 15} => #{v}" end

# set dependency package paths and generate native INI file
cmake_prefix_path = [ options[:hipo] ].compact
pkg_config_path   = [ options[:fmt]  ].compact.map{ |path| path += '/lib/pkgconfig' }
def singleQuotes(arr)
  "#{arr}".gsub /"/, "'"
end
native_ini = options[:build] + '.ini'
native_file = File.open native_ini, 'w'
native_file.puts """[built-in options]
cmake_prefix_path = #{singleQuotes cmake_prefix_path}
pkg_config_path = #{singleQuotes pkg_config_path}
"""
native_file.close

# clean and purge
def rmDir(dir,obj='files')
  print "\nRemove #{obj} at #{dir} ?\n[y/N] > "
  if $stdin.gets.downcase.chomp=="y"
    FileUtils.rm_rf dir, secure: true, verbose: true
  else
    puts "Not removing #{obj}!"
  end
end
if options[:clean] or options[:purge]
  rmDir options[:build], 'buildsystem'
end
if options[:purge]
  rmDir options[:install], 'installation'
end

# make the installation directory and get its realpath
FileUtils.mkdir_p options[:install]
prefix = File.realpath options[:install]

# print and run a command
def runCommand(cmd)
  puts "[+++] #{cmd}"
  system cmd or raise "FAILED: #{cmd}"
end

# meson commands
meson = {
  :setup => [
    'meson setup',
    "--prefix #{prefix}",
    "--native-file #{native_ini}",
    options[:build],
    SourceDir,
  ],
  :config => [
    'meson configure',
    "--prefix #{prefix}",
    "--native-file #{native_ini}",
    options[:build],
  ],
  :install => [
    'meson install',
    "-C #{options[:build]}",
  ],
}.map{ |k,v| [k, v.join(' ')] }.to_h

# run meson
if Dir.exists?(options[:build])
  runCommand "#{meson[:config]} || #{meson[:setup]}"
else
  runCommand meson[:setup]
end
runCommand meson[:install]
