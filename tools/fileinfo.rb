#!/usr/bin/ruby
# Create fileinfo for disk root directory from filenames.

CLUSTER_SIZE = 512

# Make string as a given length.
# If the string is shorter than the length, filled with spaces.
# If the string is longer than the length, cut off.
def make_str_len(str, len)
  if str.length < len
    str + ' ' * (len - str.length)
  else
    str[0...len]
  end
end

def create_fileinfo(filename, cluster)
  up = filename.upcase
  base = File.basename(up, '.*')
  ext = File.extname(up)
  ext = ext[1..-1] unless ext.empty?

  type = 0x20  # Normal file.
  time = 0
  date = 0
  size = File.size(filename)

  fileinfo = (make_str_len(base, 8) + make_str_len(ext, 3) +
              ([type] +
               [0] * 10 +
               [time,
                date,
                cluster,
                size]).pack('C11vvvV'))
  return fileinfo, size
end

def main
  cluster = 2  # Cluster starts from 2.
  table = []
  ARGV.each do |filename|
    fileinfo, size = create_fileinfo(filename, cluster)
    table.push(fileinfo)
    cluster += (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE
  end
  print table.join
end

main
