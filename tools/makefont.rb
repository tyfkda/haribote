# coding: utf-8
def read_font(f)
  char_data = Array.new(256)
  until f.eof
    line = f.readline
    if line =~ /^char 0x([0-9a-fA-F]+)/
      char_data[$1.hex] = read_char1(f)
    end
  end
  return char_data
end

def read_char1(f)
  (0...16).map do |i|
    line = f.readline
    (0...8).map {|i| line[i] == '*' ? 1 : 0}.inject(0) {|acc, x| acc * 2 + x}
  end
end

def output_font(f, font_data)
  font_data.each do |dat|
    unless dat
      dat = [0] * 16
    end
    f.puts "  { #{dat.map {|x| sprintf("0x%02x", x)}.join(', ')} },"
  end
end

font = read_font($stdin)
puts "const unsigned char fontdata[256][16] = {"
output_font($stdout, font)
puts "};"
