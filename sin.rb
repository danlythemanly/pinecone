
def sine_byte(x)
  (Math.sin(x) * 127 + 128).truncate
end

#samples=2272.0
#samples=128.0
samples=215.0
#samples=128.0
#samples=71.0

puts "#define LENGTH #{samples}"
puts "PROGMEM const uint8_t wav[LENGTH] = {"

#inc = (2 * Math::PI) / 256.0
inc = (2 * Math::PI) / samples
val = 0
num = 0
loop do
  8.times do
    printf "0x%02x, ", sine_byte(val)
    val += inc
    num += 1
    break if num == samples
  end
  printf "\n"
  break if num == samples
end
printf "};\n"

# inc = (2 * Math::PI) / 256.0
# puts inc
# val = inc
# 256.times do
#   puts "#{val}: #{Math.sin(val)}"
#   val += inc
# end
