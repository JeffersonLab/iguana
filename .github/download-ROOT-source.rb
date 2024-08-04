#!/usr/bin/env ruby
# download the ROOT source code

if ARGV.empty?
  $stderr.puts "USAGE: #{$0} [tag]"
  exit 2
end
TAG  = ARGV.first
REPO = 'root-project/ROOT'

info = Hash.new
case TAG
when 'latest'
  info = {
    :api_url   => "https://api.github.com/repos/#{REPO}/releases/latest",
    :jq_filter => '.tarball_url',
  }
else
  info = {
    :api_url   => "https://api.github.com/repos/#{REPO}/tags",
    :jq_filter => ".[] | select (.name==\"#{TAG}\") | .tarball_url",
  }
end

api_args = [
  '--silent',
  '-L',
  '-H "Accept: application/vnd.github+json"',
  '-H "X-GitHub-Api-Version: 2022-11-28"',
  info[:api_url],
]
cmd = "curl #{api_args.join ' '} | jq -r '#{info[:jq_filter]}'"
puts """API call:
#{'='*82}
#{cmd}
#{'='*82}"""

error = Proc.new do |msg|
  $stderr.puts "ERROR: #{msg}"
  exit 1
end

payload = `#{cmd}`
error.call 'GitHub API call failure' unless $?.success?
error.call "GitHub API call returned empty string, perhaps tag '#{TAG}' does not exist?" if payload.empty?
url = payload.chomp
puts "TARBALL URL = #{url}"

puts "Downloading ROOT version '#{TAG}'..."
exec "wget -nv --no-check-certificate --output-document root.tar.gz #{url}"
