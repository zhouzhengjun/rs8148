
.  ./env.sh
cd ./scripts/

./load_vpss.sh
./load_video.sh
./load_c6xdsp.sh
./osa_kermod_load.sh

./wait_cmd.sh s m3vpss  ${REMOTE_DEBUG_ADDR}
./wait_cmd.sh s m3video ${REMOTE_DEBUG_ADDR}
./wait_cmd.sh s c6xdsp  ${REMOTE_DEBUG_ADDR}

cd -

# load module fb
insmod ./kermod/vpss.ko i2c_mode=1 sbufaddr=${HDVPSS_SHARED_MEM}
insmod ./kermod/ti81xxfb.ko vram=0:8M,1:8M,2:4M
insmod ./kermod/ti81xxhdmi.ko 

sleep 1

#
# enable range compression in HDMI 0..255 to 16..235.
# This is needed for consumer HDTVs
#
./bin/mem_rdwr.out --wr 0x46c00524 2


