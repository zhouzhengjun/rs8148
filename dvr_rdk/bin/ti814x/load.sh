
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
insmod ./kermod/vpss.ko i2c_mode=0 mode=hdmi:1080p-60 sbufaddr=${HDVPSS_SHARED_MEM}
#insmod ./kermod/vpss.ko i2c_mode=0 mode=hdmi:1080p-60 sbufaddr=${HDVPSS_SHARED_MEM}
#linux mem should be 384M
insmod ./kermod/ti81xxfb.ko vram=0:120M,1:4M,2:4M
#below code starts SGX engine
devmem2 0x48180F04 w 0x0     
devmem2 0x48180900 w 0x2   
devmem2 0x48180920 w 0x2 

#insmod ./kermod/ti81xxvo.ko
#insmod ./kermod/tvp7002.ko
#insmod ./kermod/ti81xxvin.ko

insmod ./kermod/ti81xxhdmi.ko

sleep 1

#
# enable range compression in HDMI 0..255 to 16..235.
# This is needed for consumer HDTVs
#
./bin/mem_rdwr.out --wr 0x46c00524 2


