#! /bin/sh

export LD_LIBRARY_PATH=$PWD/firmware
export QT_QWS_FONTDIR=/usr/lib/fonts

dvrapp_dir="./bin"
mkdir -p $dvrapp_dir/dvrapp_cfg

./init.sh n
./load.sh
$dvrapp_dir/dvrmain -qws hdmi0 hdmi1 &
