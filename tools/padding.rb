#!/usr/bin/ruby
# Put padding bytes '\0' after file entity

require 'optparse'

def parse_int(str)
  if str =~ /^0x(.*)/
    $1.to_i(16)
  else
    str.to_i
  end
end

def main
  padding_size = 512
  padding_only = false
  warn_size = false

  opt = OptionParser.new
  opt.on('-s size', 'Set padding size') {|v| padding_size = parse_int(v)}
  opt.on('-p', 'Output padding only') {|v| padding_only = true}
  opt.on('-c', 'Exit if file is longer than padding size') {|v| warn_size = true}
  opt.parse!(ARGV)

  ARGV.each do |filename|
    size = File.size(filename)
    if warn_size && size > padding_size
      $stderr.puts "#{filename}: longer than #{padding_size} bytes"
      exit 1
    end
    mod = size % padding_size
    pad = mod == 0 ? 0 : padding_size - mod

    print File.read(filename) unless padding_only
    print("\0" * pad)
  end
end

main
