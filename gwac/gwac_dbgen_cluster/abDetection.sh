#!/bin/bash

while true
do
    #python ../gwac_dbgen/pipeline.py 1 1 &&
    file=`ls /home/wamdm/gwac/catalog.csv/*`
    echo "wamdm80 ${file}" > /tmp/Squirrel_pipe_test
    sleep 2
    truncate -s 0 /home/wamdm/gwac/nohup.out
    sleep 1
done

