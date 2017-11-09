#!/bin/bash

sumport=1984
masterhost="wamdm80"
abhost=$1
abnum=$2

echo "abnormal $abhost $abnum" | nc -q 0 $masterhost 1984
echo "finished."
exit 0
