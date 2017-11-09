#!/bin/bash
recoverRedisSleepTime=$1
cd $HOME/redis-3.2.5
while true
do
ip=`ps ax | grep [r]edis-server | awk 'FNR==1{print $6}'`
if [ $ip == "" ]
then
   echo "The avilible ip is non-exist."
   exit 0
fi
ipset=`redis-cluster-tool/redis-cluster-tool -a $ip -C "cluster_cluster_info 11" | awk 'BEGIN{i=1} {split($1,a,"[");split(a[2],b,"]"); err[i]=$3;ms[i]=a[1];ip[i++]=b[1]} END{ for(j=1;j<i;j++) if(err[j]=="error") {if(ms[j]=="master" && ms[j+1]=="" || ms[j]=="slave") {printf("%s\n",ip[j])} else print "" }}'`

cd utils/create-cluster 
for LINE in `echo $ipset`
do
        #echo $LINE
	echo $(date +%Y%m%d+%X) >> redisHostCrashLog
	echo $LINE >> redisHostCrashLog
	./create-cluster renode $LINE $ip
	echo $LINE has been restarted
done
sleep $recoverRedisSleepTime
cd ../..
done

