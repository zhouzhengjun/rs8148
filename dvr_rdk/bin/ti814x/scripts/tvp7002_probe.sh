
# script to probe TVP7002 at expected addresses and print the TVP7002 chip ID

echo ' '
echo ' *** TVP7002 probe : START ***'
echo ' '
echo ' TI VS EVM : TVP7002 device address : 0x5d, 0x00, 0x00, 0x00'
echo ' TI DVR    : TVP7002 device address : 0x5d, 0x??, 0x??, 0x??'
echo ' '
echo ' Device found     : I2C (0xXX): 0x08 = 0x00'
echo '                    I2C (0xXX): 0x09 = 0x00'
echo ' '
echo ' Device NOT found : I2C (0xXX): Read ERROR !!! (reg[0x08], count = 2)'
echo ' '

../bin/i2c_rdwr.out -r 5d 0x8 2	

echo ' '
echo ' *** TVP7002 probe : END ***'
echo ' '


echo ' *** BOARD ID probe : START ***'
../bin/i2c_rdwr.out -r 51 0x8 2
echo ' *** BOARD ID probe : END ***'
echo ' '

echo ' *** PCF8575 probe : START ***'
../bin/i2c_rdwr.out -r 21 0x8 2
echo ' *** PCF8575 probe : END ***'
echo ' '

echo ' *** SiI1161 probe : START ***'
../bin/i2c_rdwr.out -r 3b 0x8 2
echo ' *** BOARD ID probe : END ***'
echo ' '

echo ' *** LCD TOUCH SCREEN probe : START ***'
../bin/i2c_rdwr.out -r 40 0x8 2
../bin/i2c_rdwr.out -r 41 0x8 2
../bin/i2c_rdwr.out -r 4a 0x8 2
echo ' *** LCD TOUCH SCREEN probe : END ***'
echo ' '



