BUILD_DIR=$(pwd)



cd ../../

KERNEL=$(pwd)

cd $BUILD_DIR

cd compat-wireless-2.6

export KLIB_BUILD=$KERNEL
export KLIB=/home/$USER/targetfs 
./scripts/driver-select wl12xx

make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-  
 make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- install-modules 


cd $BUILD_DIR

cd compat-bluetooth

export KLIB_BUILD=$KERNEL
export KLIB=/home/$USER/targetfs
./scripts/driver-select bt


make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-
 make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- install-modules




