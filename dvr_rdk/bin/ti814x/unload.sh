
.  ./env.sh
rmmod ./kermod/ti81xxfb.ko

#rmmod ./kermod/ti81xxvo.ko
#rmmod ./kermod/tvp7002.ko
#rmmod ./kermod/ti81xxvin.ko

rmmod ./kermod/ti81xxhdmi.ko 
rmmod ./kermod/vpss.ko 
rmmod ./kermod/osa_kermod.ko 

cd ./scripts/

./send_cmd.sh x c6xdsp ${REMOTE_DEBUG_ADDR}
./wait_cmd.sh e c6xdsp ${REMOTE_DEBUG_ADDR}

./send_cmd.sh x m3video ${REMOTE_DEBUG_ADDR}
./wait_cmd.sh e m3video ${REMOTE_DEBUG_ADDR}

./send_cmd.sh x m3vpss ${REMOTE_DEBUG_ADDR}
./wait_cmd.sh e m3vpss ${REMOTE_DEBUG_ADDR}

cd -

./bin/fw_load.out shutdown DSP
./bin/fw_load.out shutdown VIDEO-M3 
./bin/fw_load.out shutdown VPSS-M3 



