BUILD_DIR=$(pwd)



cd ../../

KERNEL=$(pwd)

cd $BUILD_DIR

cd compat-wireless-2.6

export KLIB_BUILD=$KERNEL
export KLIB=/home/$USER/targetfs 

make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-   clean


cd $BUILD_DIR

cd compat-bluetooth

export KLIB_BUILD=$KERNEL
export KLIB=/home/$USER/targetfs

make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi-  clean




