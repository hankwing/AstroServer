# AServ
## 系统介绍
本系统是天文大数据实时层处理的模拟系统。数据由模拟生成器产生，经过本地交叉证认，将交叉证认的结果发送给异常星检测模块，异常星检测模块将结果发送给前端展示。
## 硬件环境
目前模拟的望远镜数量为20个，每台望远镜下配置一台机器。其中一台机器为主控节点(10.0.83.68), 其余机器为worker节点( 10.0.83.69~10.0.83.78). 一般来说只需要连接主控节点来控制作业运行。连接前要配置中科院网络中心VPN。
## 异常检测模块
jaguar目录为异常检测模块与前端展示模块。
运行AServ时，首先需要开启异常检测模块，异常检测模块会通过管道监听交叉证认的结果并处理，处理后会将处理结果发送给前端展示。运行步骤：
1.	cd ~/jaguar/jaguar-go/bin目录
2.	执行命令：./jaguar --nofileoutput –nocache，等待数据load完成
3.	可打开浏览器输入10.0.83.68:8080可看到前端展示结果，其中异常星以黄色标记。

## 模拟数据模块
gwac/gwac_dbgen为模拟数据生成模块。这一模块实际运行时与交叉证认模块一起运行。

## 交叉证认模块
gwac/Squirrel为交叉证认模块源码。  
运行步骤为：
1.	进入目录:cd ~/gwac/gwac_dbgen_cluster/
2.	生成模板星表并开启交叉证认线程：./genTemplateStartSquirrel.sh
3.	生成模拟数据，1000代表模拟的次数，每15秒一次：./sumLineAndDataSize.sh 1000
4.	停止：./stopGen.sh force

## 容器运行方式
目前已经将三个模块封装于容器内，可用如下命令在安装有容器的机器中运行AServ：
1. 创建volume:
>docker volume create catalog.csv  
docker volume create jaguar_cache  
docker volume create jaguar_data  
docker volume create jaguar_pipe  
docker volume create jaguar_result  
docker volume create squirrel_pipe  

2. 启动模拟数据生成模块
> docker run -d -it --name gwac_dbgen -v catalog.csv:/catalog.csv wamdm/gwac_dbgen:developed

3. 启动异常检测及前端展示模块
> docker run -d -p 9090:8080 -it --name jaguar -v jaguar_pipe:/home/wamdm/jaguar/jaguar-go/pipe -v jaguar_cache:/home/wamdm/jaguar/jaguar-go/cache -v jaguar_result:/home/wamdm/jaguar/jaguar-go/result -v jaguar_data:/home/wamdm/jaguar/jaguar-go/data wamdm/jaguar:developed

4. 启动交叉证认模块
> docker run -d -it --name squirrel -v squirrel_pipe:/tmp -v catalog.csv:/catalog.csv -v jaguar_pipe:/home/wamdm/jaguar/jaguar-go/pipe wamdm/squirrel:developed

5. 生成一次模拟数据
> docker exec gwac_dbgen python pipeline.py 1 1
 
6. 对模拟数据进行交叉证认，并将结果发送给异常检测，检测结果前端展示
> docker exec squirrel bash -c "echo '/catalog.csv/`docker exec squirrel ls /catalog.csv`' > /tmp/Squirrel_pipe_test"

7. 停止容器
> docker stop gwac_dbgen  
docker rm gwac_dbgen  
docker stop squirrel  
docker rm squirrel  
docker stop jaguar  
docker rm jaguar  

## 联系方式
Email: hankwing@hotmail.com

