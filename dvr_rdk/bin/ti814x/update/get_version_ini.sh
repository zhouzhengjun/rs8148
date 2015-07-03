#!/bin/sh

if [ -f version.ini ]
then 
rm version.ini
fi

wget  http://192.168.1.101/bell/update/version.ini
chmod 777 version.ini


