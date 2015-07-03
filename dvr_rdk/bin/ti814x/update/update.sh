#!/bin/sh

cat  version.ini
while read line; do
name=`echo $line|awk -F '=' '{print $1}'`
value=`echo $line|awk -F '=' '{print $2}'`
case $name in
"version")
VERSION=$value
;;
"date")
DATE=$value
;;
*)
;;
esac
done < version.ini
echo $VERSION
echo $DATE
if [ ! -f $1 ]
then
wget http://192.168.1.101/bell/update/$1
fi
if [ ! -d $version]
then
rm -r $version
fi
tar -zxf $1 

cd $VERSION

make uninstall
make install

if [ -f uImage ]
then
flash_eraseall /dev/mtd3 
nandwrite -p /dev/mtd3 uImage 
fi

if [ -f u-boot.bin ]
then
flash_eraseall /dev/mtd1
nandwrite -p /dev/mtd1 u-boot.bin
fi

if [ -f u-boot-min.bin ]
then
flash_eraseall /dev/mtd0
nandwrite -p /dev/mtd0 u-boot-min.bin
fi

