
# script to probe TVP5158 at expected addresses and print the TVP5158 chip ID

echo ' '
echo ' *** TVP5158 probe : START ***'
echo ' '
echo ' TI VS EVM : TVP5158 device address : 0x58, 0x5a, 0x5c, 0x5e'
echo ' TI DVR    : TVP5158 device address : 0x58, 0x59, 0x5a, 0x5b'
echo ' '
echo ' Device found     : I2C (0xXX): 0x08 = 0x51'
echo '                    I2C (0xXX): 0x09 = 0x58'
echo ' '
echo ' Device NOT found : I2C (0xXX): Read ERROR !!! (reg[0x08], count = 2)'
echo ' '

../bin/i2c_rdwr.out -r 58 0x8 2	
../bin/i2c_rdwr.out -r 5a 0x8 2
../bin/i2c_rdwr.out -r 5c 0x8 2
../bin/i2c_rdwr.out -r 5e 0x8 2
../bin/i2c_rdwr.out -r 59 0x8 2
../bin/i2c_rdwr.out -r 5b 0x8 2

echo ' '
echo ' *** TVP5158 probe : END ***'
echo ' '