#!/bin/bash
 
#genFileNum=$1
masterhost=$1
ccdno=$2
abnum=$3
redisIPBackup=$4
templateTable=$5
sourceFilePath=$6
threadNumber=$7
getTemplateFlag=$8
#sleeptime=15
post=1984
stoppost=1986
tmp=""
hostname=$(hostname)
maxLogSize=10485760
maxTimeOut=12
#redisIP=`ps -ef | grep [r]edis-server | awk BEGIN{i=0}'{a[i++]=$9} END{srand();j=int(rand()*100%NR);print a[j]}'`
echo "exit" | nc -q 0 $hostname $stoppost #Terminate the previous killSend.sh
nohup ./killSend.sh $$ $masterhost & 
#for i in $(seq 1 $genFileNum)
#do
#t1=$((time -p (python ../gwac_dbgen/pipeline.py $ccdno >dtmp)) 2>&1 | head -1 |awk '{print $2}')
#eval $(python ../gwac_dbgen/pipeline.py $ccdno | awk -F '{a=$1} END{print "tmp="a}')

pipelinedata=`python ../gwac_dbgen/pipeline.py $ccdno $abnum`
nohup scp -r ../catalog.csv/* ../data_backup &
###############################+ test template data to put in redis without cross validation. 
#dt=`date "+%Y-%m-%d %H:%M:%S"`  #annotate pipelinedata=`python ../gwac_dbgen/pipeline.py $ccdno $abnum`and the catalog.csv/$sourceFile and template_table/template are the same.
#pipelinedata="0 0 $dt"
##############################+
resultNum=0
abStarNum=0
stopinfo="stop $hostname $resultNum $abStarNum"
redisIP=`ps -ef | grep [r]edis-server | awk 'FNR==1{print $9}'`

if [ "$redisIP" == "" ]
   then
    redisIP=$redisIPBackup
fi

if [ "$getTemplateFlag" == "gt" ]
then
       pgrep "Squirrel" | while read Spid
         do
            kill $Spid
         done
      ./genTemplateTable.sh
      rm -rf /tmp/Squirrel_pipe_test # delete Squirrel pipe file to prevent unknown reason to block write to Squirrel_pipe_test at the first time (echo "$hostname $pipelinedata ../$sourceFile" > /tmp/Squirrel_pipe_test)
nohup ../Squirrel/Debug/Squirrel -times 1 -redisHost $redisIP -method plane -grid 4,4 -errorRadius 1.5 -searchRadius 50 -ref  ../$templateTable -threadNumber $threadNumber -width 3016 -height 3016 -terminal &
else
##########jedis client
#sourceFile=$sourceFilePath/$(ls ../$sourceFilePath)
#/home/wamdm/jdk1.7.0_79/bin/java -jar ../JedisTest.jar ../$sourceFile 5 1 >>1.txt 2>&1
#redisMonitorData="1 1 1"
##########jedis client
sourceFile=$sourceFilePath/$(ls ../$sourceFilePath)
echo "$hostname $pipelinedata $HOME/gwac/$sourceFile" > /tmp/Squirrel_pipe_test
sumresult=""
eval $(echo $redisIP | awk -F ":" '{i=$1;p=$2} END{print "ip="i";port="p}')

for((timeout=1;timeout<=$maxTimeOut;timeout++))
do
   sleep 0.5
   result=`$HOME/redis-3.2.5/src/redis-cli -h $ip -p $port -c lpop $hostname`
   if [ "$result" == "" ]
     then
       continue
     fi
   echo "$result" | ./send.py $masterhost $post
   sumresult="$sumresult\n$result"
   eval $(echo "$result" | awk -v abs=$abStarNum '{abs+=$NF} END{print "abStarNum="abs}')
   resultNum=$(($resultNum+1))
  # if [ $resultNum -eq 1 ]  # break until receiving one result, but need a large maxTimeOut. E.g., maxTimeOut=10000000
  #   then 
  #     break
  #   fi
done
stopinfo="stop $hostname $resultNum $abStarNum"
sumresult=`echo $sumresult | cut -c 3-`

if [ "$sumresult" == "" ]
then 
stopinfo="$stopinfo timeout"
else
#### log
     newLogName=`ls logs/ | grep "slave" | tail -1`
if [ "$newLogName" == "" ]
then
     newLogName="slave_${hostname}_1_`date "+%Y-%m-%d-%H:%M:%S"`"
     echo -e "PipelineLine Pipelinesize PipelineDate RedisTime(s) RedisStorageLine CrossCertifiedErrorLine\n0 0 0 0 0 0" > logs/$newLogName
fi
     logSize=`ls -lk logs/$newLogName | awk '{print $5}'`
     if [ $logSize -le $maxLogSize ]
     then
       echo -e $sumresult >>logs/$newLogName 
     else
     newLogNum=$(echo $newLogName | awk -F "_" '{print $3}')
     newLogNum=$(($newLogNum+1))
     newLogName="slave_${hostname}_${newLogNum}_`date "+%Y-%m-%d-%H:%M:%S"`"
     echo -e "PipelineLine Pipelinesize PipelineDate RedisTime(s) RedisStorageLine CrossCertifiedErrorLine\n$sumresult" > logs/$newLogName
     fi 
   ####log
fi
fi
#echo $(hostname) >>dtmp
#t2=$((time -p (cat dtmp | nc -q 0 $masterhost $post)) 2>&1 | head -1 |awk '{print $2}')
#timeleft=`echo "$sleeptime-($t1+$t2)"| bc`
#sleep $timeleft
#done
./send.py $masterhost $post "$stopinfo"
#pkill killSend.sh >/dev/null 2>&1 # easily cause redis-server terminates unexpectedly
#lsof -Pnl +M -i4 | grep $stoppost | awk '{print $2}' | xargs kill >/dev/null 2>&1
#rm -rf dtmp
