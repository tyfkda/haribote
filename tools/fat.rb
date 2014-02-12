#!/usr/bin/ruby
# Create FAT (File Allocation Table)

# Layout given files sequentially.

CLUSTER_SIZE = 512

def construct_fat(filenames)
  fat = [0xff0, 0xfff]  # Cluster 0, 1 are reserved.
  cluster = 2
  ARGV.each do |filename|
    size = File.size(filename)
    cluster_count = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE
    (cluster_count - 1).times do |i|
      fat.push(cluster + i + 1)  # Indicate next cluster.
    end
    fat.push(0xfff)  # End mark.
    cluster += cluster_count
  end
  return fat
end

def output_fat(fat)
  if fat.size.odd?
    fat.push(0)
  end

  s = (0...(fat.size / 2)).map do |i|
    n1 = fat[i * 2 + 0]
    n2 = fat[i * 2 + 1]
    [n1 & 0xff, ((n2 & 0x0f) << 4) | (n1 >> 8), n2 >> 4]
  end.flatten
  print s.pack('C*')
end

def main
  output_fat(construct_fat(ARGV))
end

main
