chmod +x *.sh
chmod +x ./bin/*.out
chmod +x ./scripts/*.sh

#Set the environment variables used by other scripts.
#This should be first script executed
.  ./env.sh
./validate.sh

if test $? -eq "1"
  then 
    echo "Kernel bootargs validated"
  else
    echo "Kernel bootargs mismatch.!!!!ABORTING!!!"
    exit 0
fi

amixer cset name='PCM Playback Volume' 100%,100%

rmmod ./kermod/syslink.ko 2> /dev/null
rmmod ./kermod/osa_kermod.ko 2> /dev/null
rmmod ./kermod/TI81xx_hdmi.ko 2> /dev/null

killall fw_load.out 2> /dev/null
killall remote_debug_client.out 2> /dev/null

sleep 1

./bin/remote_debug_client.out ${REMOTE_DEBUG_ADDR} &

insmod ./kermod/syslink.ko TRACE=1 TRACEFAILURE=1 

sleep 1

#
# Format of the system priority setting utility for setting bandwidth regulator
#
# ./bin/sys_pri.out --L3-bw-reg-set <L3-bw-reg-initiator-name> <L3-pressure-High> <L3-pressure-Low> <L3-Bandwidth> <L3-Watermark-cycles>
#
# "L3-bw-reg-initiator-name" can be HDVICP0 or HDVICP1 or HDVICP2 or other initiators
# "L3-pressure-High" can be 0 (low), 1 (medium), 3 (high)
# "L3-pressure-Low"  can be 0 (low), 1 (medium), 3 (high)
# "L3-Bandwidth" is in MB/s
# "L3-Watermark-cycles" is in bytes
#
# See also ./bin/sys_pri.out --help for more details
#
# IVA-HD BW requlator programing.
# TODO. THIS IS CURRENTLY HARDCODED.
# THIS SHOULD CALCULATED FROM EXPECTED IVA BANDWIDTH USAGE
#
# IVA-HD0, IVA-HD1, IVA-HD2
#
./bin/sys_pri.out --L3-bw-reg-set HDVICP0 3 3 2000 4000
./bin/sys_pri.out --dmm-pri-set DUCATI 0
./bin/sys_pri.out --dmm-pri-set HDVICP0 2
