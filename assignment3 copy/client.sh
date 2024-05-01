#! /bin/bash                                                                    
PORT=$1
NUM_PLAYERS=$2

sleep 1 
for ((i=1; i<=$NUM_PLAYERS; i++))
do
./play VM-16-6-ubuntu $PORT & 
done

wait