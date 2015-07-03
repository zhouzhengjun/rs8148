
export DISPLAY=:0
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/lib

#Increase network max buffer size
sysctl -w net.core.rmem_max=256960
sysctl -w net.core.wmem_max=256960
sysctl -w net.core.rmem_default=256960
sysctl -w net.core.wmem_default=256960

cd /opt/dvr_rdk/ti814x
./init.sh
./load.sh

