#!/usr/bin/env ruby

require 'optparse'
require 'fileutils'

SourceDir = __dir__

# parse options
options = {
  build:   "#{Dir.pwd}/build-iguana",
  install: "#{Dir.pwd}/iguana",
  hipo:    ["#{Dir.pwd}/hipo", ENV['HIPO']].compact.join(';'),
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
parser.on("-h", "--hipo [HIPO DIR]", "Path to HIPO installation", "Default: #{options[:hipo].split(';')[0]}", "if not found, tries env var $HIPO")
parser.separator 'TROUBLESHOOTING:'
parser.on("--clean", "Remove the buildsystem at [BUILD DIR] beforehand")                 
parser.on("--purge", "Remove the installation at [INSTALL DIR] beforehand")                 
parser.parse!(into: options)

# print the options
puts "SET OPTIONS:"
options.each do |k,v| puts "#{k.to_s.rjust 15} => #{v}" end

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
def exe(cmd)
  puts "[+++] #{cmd}"
  system cmd or raise "FAILED: #{cmd}"
end

# set a build option
def buildOpt(key,val)
  "-D#{key}='#{val}'"
end

# meson commands
meson = {
  :setup => [
    'meson setup',
    "--prefix #{prefix}",
    buildOpt('hipo', options[:hipo]),
    options[:build],
    SourceDir,
  ],
  :config => [
    'meson configure',
    "--prefix #{prefix}",
    buildOpt('hipo', options[:hipo]),
    options[:build],
  ],
  :install => [
    'meson install',
    "-C #{options[:build]}",
  ],
}.map{ |k,v| [k, v.join(' ')] }.to_h

# run meson
if Dir.exists?(options[:build])
  exe "#{meson[:config]} || #{meson[:setup]}"
else
  exe meson[:setup]
end
exe meson[:install]
