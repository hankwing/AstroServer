#!/bin/bash

fpid=$1
masterhost=$2
stoppost=1986
hostname=$(hostname)
ps ax | grep [.]/sendStaDataPerNode.sh | awk '{print $1}' | while read pid
do
  if [ $fpid -ne $pid ]
    then
        kill $pid
    fi
done
#lsof -Pnl +M -i4 | grep $stoppost | awk '{print $2}' | xargs kill >/dev/null 2>&1
tmp=`nc -lp $stoppost`
#echo "$tmp" >>1.txt
if [ "$tmp" == "force" ]
then
spid=`ps -ax | awk '{ print $1 }' | grep $fpid`
 while [ "$fpid" == "$spid" ]
   do  
     kill $fpid
     spid=`ps -ax | awk '{ print $1 }' | grep $fpid`
   done

    pgrep "Squirrel" | while read line
     do
       kill $line
     done
 echo "$hostname has stopped!" | ./send.py $masterhost $stoppost
exit 0
elif [ "$tmp" == "kill" ]
then
 pgrep "Squirrel" | while read line
  do
    kill $line
  done
exit 0
elif [ "$tmp" == "exit" ]
then 
exit 0
else
echo "$hostname's \"force\" is error, and the real message is $tmp." | ./send.py $masterhost $stoppost
fi
