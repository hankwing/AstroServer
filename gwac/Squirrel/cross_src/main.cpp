/*
 ######################################################################
 ## Time-stamp:
 ## Filename:      $Name:  $
 ## Version:       $Revision: 1.2 $
 ## Author:        Yang Xu <yxuctgu@gmail.com>
 ## Purpose:       cross match of astrometry.
 ## CVSAuthor:     $Author: cyxu $
 ## Note:
 #-
 ## $Id: main.cpp,v 1.2 2012/04/16 08:11:15 cyxu Exp $
 #======================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <sys/time.h>
#include <sys/file.h>
#include <fstream>

#include "function.h"
#include "StarFileFits.h"
#include "CrossMatchSphere.h"
#include "CrossMatch.h"
#include "NamedPipe.h"
#include "acl_cpp/lib_acl.hpp"

#define TestCrossMatch1

void showHelp();
void setDefaultValue();
int getStrValue(char *src, char *name, char *value);
int parsePara(int argc, char** argv);
void mainSphere(char *refFile, char *objFile, char *outFile);
void mainPlane(char *refFile, char *objFile, char *outFile);
void mainSphereTest(char *refFile, char *objFile, char *outFile);
void mainPlaneTest(char *refFile, char *objFile, char *outFile);

int matchOT; //is match ot in database, may be need acceleration
int showResult; //0输出所有结果，1输出匹配的结果，2输出不匹配的结果
int printResult; //0将匹配结果不输出到终端，1输出到终端
int showProcessInfo; //0不输出处理过程信息，1输出
int fitsHDU = 3; //fitsHDU=3: read fits file from the 3rd hdu
int useCross; //use cross method
double minZoneLength; //the min length of the zone's side
double searchRadius;
double areaBox; //判断两颗星是一颗星的最大误差，默认为20角秒

int wcsext = 2;
float magErrThreshold = 0.05; //used by getMagDiff
float minZoneLen = areaBox * 10;
float searchRds = areaBox;

int dbConfigInCommandLine;
int areaWidth;
int areaHeight;
float planeErrorRedius;
int fluxRatioSDTimes; //factor of flux filter

int gridX; //对星表进行分区计算fluxratio，gridX为分区X方向上的个数。
int gridY; //对星表进行分区计算fluxratio，gridY为分区Y方向上的个数。

int cpu;
int method;
char *cmdDbInfo;
char *refFile;
char *objFile;
char *outFile;
char *configFile;
//StoreDataPostgres *dataStore;
StarFileFits *refStarFile = NULL, *objStarFile = NULL;
Partition *zones;

acl::redis_client_cluster *cluster;
//acl::redis_client *conn;
static const char* redisHost = (char*) malloc(sizeof(char*) * 30);
int crossTime = 0;
int threadNum = 5;
int sendNumControl = 0;
int sendNumCount = 0;
const string abnormalDetNamedFile = "/home/wamdm/jaguar/jaguar-go/pipe/namedpipe";
// mutex controing threads


void setDefaultValue() {

	method = PLANE_METHOD;
	cpu = 1;
	gridX = 1;
	gridY = 1;
	areaWidth = 3016;
	areaHeight = 3016;
	matchOT = 0;
	areaBox = 1.5;
	minZoneLength = areaBox;
	searchRadius = 50;
	fitsHDU = 3;
}

void RedisInit() {
	// init socket module for windows
	if (strlen(redisHost) < 2) {
		printf("error: you must specify redis Host address through -redisHost option\n");
		exit(0);
	}
	acl::acl_cpp_init();

	//const char* redis_addr = "127.0.0.1:6379";
	int conn_timeout = 10, rw_timeout = 10, max_conns = 1000;

	// the redis client connection
	//conn = new acl::redis_client(redisHost, conn_timeout, rw_timeout);
	// declare redis cluster ojbect

	cluster = new acl::redis_client_cluster();
	cluster->set_retry_inter(2);
	cluster->set_redirect_max(10);
	cluster->set_redirect_sleep(1000);

	cluster->init(NULL, redisHost, max_conns, conn_timeout, rw_timeout);

	//cluster->set(redisHost, conn_timeout, rw_timeout, max_conns);
}

static void * crossThread( void * command) {

	string commandString = (char *) command;
	string filePath = commandString.substr(commandString.find_last_of(" ") + 1);
	string keyString = commandString.substr(0, commandString.find_first_of(" "));
	printf("cross target star file:%s\n", filePath.c_str());
	commandString = commandString.substr(0, commandString.find_last_of(" "));
	memset(command, '\0', 1024);

	if (method == PLANE_METHOD) {
		if (areaWidth == 0 || areaHeight == 0) {
			printf("in plane coordinate mode, must assign \"-width\" and \"-height\"\n");
			return 0;
		}
		//#ifndef TestCrossMatch
		//mainPlane(refFile, objFile, outFile);

		//printf("starting plane cross match...\n");
		struct timeval start, end;
		gettimeofday(&start, NULL);

		//dataStore->store(refStarFile, 1);

		StarFileFits *objStarFile = new StarFileFits(filePath.c_str(), areaBox, fitsHDU, wcsext,
				fluxRatioSDTimes, magErrThreshold, gridX, gridY);
		objStarFile->readStar(false);
		//objStarFile->readProerty();
		objStarFile->setFieldWidth(areaWidth);
		objStarFile->setFieldHeight(areaHeight);

		CrossMatch *cm = new CrossMatch();
		cm->setFieldHeight(areaHeight);
		cm->setFieldWidth(areaWidth);
		//目前minZoneLength和searchRadius没有考虑
		// get the time now
		time_t rawtime;
		struct tm * timeinfo;
		char buffer [80];

		time (&rawtime);
		timeinfo = localtime (&rawtime);

		strftime (buffer,80,"%G_%m_%d_%H_%M_%S\n",timeinfo);
		//puts (buffer);
		// create output file for abnormal detection
		int outputFile = -1;
		if(access(abnormalDetNamedFile.c_str(), F_OK) != -1)
		{
			outputFile = open(abnormalDetNamedFile.c_str(), O_WRONLY);
		}
		// write head
		if(outputFile != -1 && write(outputFile, "start\n", 6));
		if(outputFile != -1 && write(outputFile, buffer, strlen(buffer)));
		//int outputFile = open(buffer, O_RDWR | O_CREAT, 0666);
		//int rc = flock(outputFile, LOCK_EX);	// block if another thread is holoding the lock
		//if( rc) {
		//	printf("open failed");
		//}
		cm->match(refStarFile, objStarFile, zones, areaBox, outputFile);
		if(outputFile != -1 && write(outputFile, "end\n", 4));
		close(outputFile);

		objStarFile->getMagDiff();
		objStarFile->fluxNorm();
		objStarFile->tagFluxLargeVariation();
		objStarFile->judgeInAreaPlane();
		//dataStore->store(objStarFile, 0);
		//cm->printMatchedRst(outFile, objStarFile, areaBox);
		gettimeofday(&end, NULL);
		cm->sendResultsToRedis(cluster, refStarFile, threadNum, sendNumControl);

		delete cm;
		delete objStarFile;
		//delete refStarFile;

		gettimeofday(&end, NULL);
		float time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
		ostringstream stringStream;
		stringStream << " "<< time_use/1000000 << " "
				<< objStarFile->matchedCount << " " << objStarFile->OTStarCount <<
				" " << objStarFile->abStar;
		commandString = commandString.append(stringStream.str());

		// send result to redis
		printf("%s\n", commandString.c_str());

		acl::redis cmd;
		cmd.set_cluster(cluster, 1000);
		while( cmd.rpush(keyString.c_str(), commandString.c_str(), NULL) < 0) {
			printf("insert failed\n");
			sleep(10);
		}
		cmd.clear();

	}

	return NULL;
}

/**
 * 目前minZoneLength和searchRadius没有考虑
 */
int main(int argc, char** argv) {

	if (argc == 1) {
		showHelp();
		return 0;
	}

	setDefaultValue();

	cmdDbInfo = (char*) malloc(LINE);
	memset(cmdDbInfo, 0, LINE);
	refFile = (char*) malloc(LINE);
	memset(refFile, 0, LINE);
	objFile = (char*) malloc(LINE);
	memset(objFile, 0, LINE);
	outFile = (char*) malloc(LINE);
	memset(outFile, 0, LINE);
	configFile = (char*) malloc(LINE);
	memset(configFile, 0, LINE);

	//dataStore = new StoreDataPostgres();

	if (parsePara(argc, argv) == 0) {
		return 0;
	}
	// init alluxio utils
	RedisInit();

	// refStar stay in memory
	refStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY, cluster, redisHost);

	refStarFile->readStar(true);
	//refStarFile->readStar(false);
	//refStarFile->readProerty();

	zones = new Partition(areaBox, minZoneLen, searchRds);
	zones->partitonStarField(refStarFile);

	//printf("ref stars init time is: %ld us\n",
	//		(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec));

	//scanf("%s", command);

	//int i = 0;
	//while (strcmp(command, "exit") != 0) {
	//printf("wait for input\n");
	NamedPipe pipe("/tmp/Squirrel_pipe_test");
	char * command;
	while (strcmp((command = pipe.getCommand()), "Squirrel_exit") != 0) {
		command[strlen(command) - 1] = '\0';
		// call another process thread every time receiving a signal
		pthread_attr_t crossProcessAttrs;
		pthread_t crossProcessId;
		pthread_attr_init(&crossProcessAttrs);
		// create first thread
		pthread_create(&crossProcessId, &crossProcessAttrs, crossThread, command);
		pthread_join(crossProcessId, NULL);
		if( ++sendNumCount >= threadNum ) refStarFile->isSendTemplate = true;

	}

	//dataStore->matchOTFlag = matchOT;

	//if (dbConfigInCommandLine == 0) {
	//  dataStore->readDbInfo(configFile);
	//} else {
	//  getDBInfo(cmdDbInfo, dataStore);
	//}

	free(cmdDbInfo);
	free(refFile);
	free(objFile);
	free(outFile);
	free(configFile);
	cluster->stop_monitor(true);
	//delete dataStore;

	return 0;
}

/*void mainPlane(char *refFile, char *objFile, char *outFile) {

	printf("starting plane cross match...\n");

	long start, end;
	start = clock();

	int wcsext = 2;
	float magErrThreshold = 0.05; //used by getMagDiff

	StarFileFits *refStarFile, *objStarFile;
	refStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	refStarFile->readStar(true);
	refStarFile->readProerty();
	//dataStore->store(refStarFile, 1);

	objStarFile = new StarFileFits(objFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	objStarFile->readStar(false);
	objStarFile->readProerty();
	objStarFile->setFieldWidth(areaWidth);
	objStarFile->setFieldHeight(areaHeight);

	CrossMatch *cm = new CrossMatch();
	cm->setFieldHeight(areaHeight);
	cm->setFieldWidth(areaWidth);
	//目前minZoneLength和searchRadius没有考虑
	// core code!!
	cm->match(refStarFile, objStarFile, areaBox);
	objStarFile->getMagDiff();
	objStarFile->fluxNorm();
	objStarFile->tagFluxLargeVariation();
	objStarFile->judgeInAreaPlane();
	//dataStore->store(objStarFile, 0);
	cm->printMatchedRst(outFile, objStarFile, areaBox);

	delete cm;
	delete objStarFile;
	delete refStarFile;

	end = clock();
	printf("total time is: %fs\n", (end - start) * 1.0 / ONESECOND);
}*/

/**
 * 天球坐标匹配
 * @param refFile 模板星表文件
 * @param objFile 目标星表文件
 * @param outFile
 */
void mainSphere(char *refFile, char *objFile, char *outFile) {

	printf("starting sphere cross match...\n");

	long start, end;
	start = clock();

	int wcsext = 2;
	float magErrThreshold = 0.05; //used by getMagDiffdata

	StarFileFits *refStarFile, *objStarFile;
	refStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	refStarFile->readStar(true);
	refStarFile->readProerty();
	//dataStore->store(refStarFile, 1);

	objStarFile = new StarFileFits(objFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	objStarFile->readStar(false);
	objStarFile->readProerty();

	CrossMatchSphere *cms = new CrossMatchSphere();
	//目前minZoneLength和searchRadius没有考虑
	cms->match(refStarFile, objStarFile, areaBox);
	objStarFile->getMagDiff();
	objStarFile->fluxNorm();
	objStarFile->tagFluxLargeVariation();
	objStarFile->wcsJudge(wcsext);
	//dataStore->store(objStarFile, 0);

	delete cms;
	delete objStarFile;
	delete refStarFile;

	end = clock();
	printf("total time is: %fs\n", (end - start) * 1.0 / ONESECOND);
}

/**
 * 如果返回0,则表示解析参数出错
 * @param argc
 * @param argv
 * @return 0 or 1
 */
int parsePara(int argc, char** argv) {

	int i = 0;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-method") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-method must follow a stirng\n");
				return 0;
			}
			if (strcmp(argv[i + 1], "sphere") == 0) {
				method = SPHERE_METHOD;
			} else {
				method = PLANE_METHOD;
			}
			i++;
		} else if (strcmp(argv[i], "-redisHost") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-redisHost must follow a stirng\n");
				return 0;
			}
			strcpy((char*) redisHost, argv[i + 1]);
			i++;

		} else if (strcmp(argv[i], "-threadNumber") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-redisHost must follow a integer\n");
				return 0;
			}
			threadNum = atoi(argv[i + 1]);
			i++;

		} else if (strcmp(argv[i], "-times") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-times must follow a integer\n");
				return 0;
			}
			crossTime = atoi(argv[i + 1]);
			i++;

		} else if (strcmp(argv[i], "-errorRadius") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-errorRadius must follow a number\n");
				return 0;
			}
			areaBox = atof(argv[i + 1]);
			minZoneLength = areaBox;
			searchRadius = areaBox;
			i++;
		} else if (strcmp(argv[i], "-searchRadius") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-searchRadius must follow a number\n");
				return 0;
			}
			searchRadius = atof(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-minZoneLength") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-minZoneLength must follow a number\n");
				return 0;
			}
			minZoneLength = atof(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-width") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-width must follow a number\n");
				return 0;
			}
			areaWidth = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-height") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-height must follow a number\n");
				return 0;
			}
			areaHeight = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-fitsHDU") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-fitsHDU must follow a numbe\n");
				return 0;
			}
			fitsHDU = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-fluxSDTimes") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-fluxSDTimes must follow a number\n");
				return 0;
			}
			fluxRatioSDTimes = atoi(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-cross") == 0) {
			useCross = 1;
		} else if (strcmp(argv[i], "-ref") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-ref must follow reference file name\n");
				return 0;
			}
			strcpy(refFile, argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-sample") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-sample must follow sample file name\n");
				return 0;
			}
			strcpy(objFile, argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-output") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-o must follow output file name\n");
				return 0;
			}
			strcpy(outFile, argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-mode") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-mode must follow cpu or gpu\n");
				return 0;
			}
			if (strcmp(argv[i + 1], "gpu") == 0) {
				cpu = 0;
			}
			i++;
		} else if (strcmp(argv[i], "-dbConfigFile") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-mode must follow file name\n");
				return 0;
			}
			strcpy(configFile, argv[i + 1]);
			dbConfigInCommandLine = 0;
			i++;
		} else if (strcmp(argv[i], "-dbInfo") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-mode must follow \"name1=value1,name2=value2...\"\n");
				return 0;
			}
			strcpy(cmdDbInfo, argv[i + 1]);
			dbConfigInCommandLine = 1;
			i++;
		} else if (strcmp(argv[i], "-show") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-show must follow all, matched or unmatched\n");
				return 0;
			}
			if (strcmp(argv[i + 1], "all") == 0) {
				showResult = 0;
			} else if (strcmp(argv[i + 1], "matched") == 0) {
				showResult = 1;
			} else if (strcmp(argv[i + 1], "unmatched") == 0) {
				showResult = 2;
			} else {
				printf("-show must follow all, matched or unmatched\n");
			}
			i++;
		} else if (strcmp(argv[i], "-terminal") == 0) {
			printResult = 1;
		} else if (strcmp(argv[i], "-matchot") == 0) {
			matchOT = 1;
			printf("matchOT=%d\n", matchOT);
		} else if (strcmp(argv[i], "-processInfo") == 0) {
			showProcessInfo = 1;
		} else if (strcmp(argv[i], "-g") == 0
				|| strcmp(argv[i], "-grid") == 0) {
			if (i + 1 >= argc || strlen(argv[i + 1]) == 0) {
				printf("-g or -grid must follow number,number\n");
				return 0;
			}
			if (2 != sscanf(argv[i + 1], "%d,%d", &gridX, &gridY)) {
				printf("-g or -grid must follow number*number\n");
				return 0;
			}
			i++;
		} else if (strcmp(argv[i], "-h") == 0
				|| strcmp(argv[i], "-help") == 0) {
			showHelp();
			return 0;
		} else if (strcmp(argv[i], "-v") == 0
				|| strcmp(argv[i], "-version") == 0) {
			printf("%s\n", VERSION);
			return 0;
		} else {
			printf("%s is unknow parameter\n", argv[i]);
			showHelp();
			return 0;
		}
	}
	return 1;
}

void showHelp() {
	printf("crossmatch v1.2 (2012 Nov 15)\n");
	printf(
			"usage: crossmatch [-method <plane> ] [-errorRadius <20>] [-width <3096>] [...]\n");
	printf(
			"usage: crossmatch [-method <sphere>] [-errorRadius <0.00556>] [...]\n");
	printf(
			"-method <sphere|plane>:     cross match method, using sphere coordinate or plane corrdinate\n");
	printf(
			"-width <number>:            on plane corrdinate partition, the max value of X axis\n");
	printf(
			"-height <number>:           on plane corrdinate partition, the max value of Y axis\n");
	printf(
			"-errorRadius <number>:      the max error between two point, unit is degree, defalut 0.005556\n");
	printf(
			"-searchRadius <number>:     the search area's radius, unit is degree, defalut equals errorRadius\n");
	printf(
			"-minZoneLength <number>:    the min length of the zone's side, unit is degree, defalut equals errorRadius\n");
	//printf("                            default, the total zone number equals the reference catalog star numbers\n");
	printf(
			"-fitsHDU <number>:          read fits file from the fitsHDU-th data area\n");
	printf("-ref <path>:                reference table path\n");
	printf("-sample <path>:             sample table path\n");
	printf("-output <path>:             output table path\n");
	//printf("\t-mode\n");
	//printf("\t\tgpu: executed by gpu\n");
	//printf("\t\tcpu: executed by cpu\n");
	printf(
			"-dbConfigFile <fileName>:   file contain database config information.\n");
	printf(
			"                            Notice: -dbInfo is prior to -dbConfigFile\n");
	printf(
			"-dbInfo \"<name1=value1,name2=value2...>\": this option include the database configure information\n");
	printf(
			"                            host=localhost, IP Address or host name\n");
	printf(
			"                            port=5432, the port of PostgreSQL use\n");
	printf("                            dbname=svomdb, database name\n");
	printf("                            user=wanmeng, database user name\n");
	printf("                            password= ,database user password\n");
	printf("                            catfile_table=catfile_id \n");
	printf("                            match_talbe=crossmatch_id \n");
	printf("                            ot_table=ot_id \n");
	printf("                            ot_flux_table=ot_flux_id \n");
	printf("-show <all|matched|unmatched>:\n");
	printf(
			"                            all:show all stars in sample table including matched and unmatched\n");
	printf(
			"                            matched:show matched stars in sample table\n");
	printf(
			"                            unmatched:show unmatched stars in sample table\n");
	printf("-terminal:                  print result to terminal\n");
	printf("-matchot:                  print result to terminal\n");
	printf(
			"-cross:                     compare zone method with cross method, find the zone method omitted stars, and output to file\n");
	printf("-processInfo:               print process information\n");
	printf(
			"-fluxSDTimes <number>:      the times of flux SD, use to filter matched star with mag\n");
	printf(
			"-g <Xnumber,Ynumber>:       the partition number in X and Y direction, used to calculate fluxratio, default is 1,1\n");
	printf("-h or -help:                show help\n");
	printf("-v or -version:             show version number\n");
	printf("-redisHost:             set Redis host:port\n");
	printf("-times:             set Redis run times number\n");
	printf("example: \n");
	printf(
			"\t1: crossmatch -method sphere -g 2,2 -errorRadius 0.006(20 arcsec) -searchRadius 0.018 -fitsHDU 2 -ref reference.cat -sample sample.cat -output output.cat -processInfo\n");
	printf(
			"\t2: crossmatch -method plane  -g 2,2 -errorRadius 10 -searchRadius 30 -width 3096 -height 3096 -fitsHDU 2 -ref reference.cat -sample sample.cat -output output.cat -processInfo\n");
	//printf("Notes: default area box is 0.005556 degree, output all result, not print to terminal, not print process information, not compare the result with cross method\n");
}

int getStrValue(char *src, char *name, char *value) {

	char *str1 = NULL;
	char *str2 = NULL;
	char *start = NULL;
	str1 = strstr(src, name);
	if (str1 == 0)
		return 0;
	start = str1 + strlen(name) + 1;
	str2 = strchr(str1, ',');
	if (str2 == 0)
		str2 = src + strlen(src);
	int len = str2 - start;
	if (len != 0) {
		strncpy(value, start, len); //there is a =, so need add 1
		value[len] = '\0';
	} else {
		strcpy(value, "");
	}
	return 1;
}

/**
 * 比较分区方式和不分区方式匹配结果的区别
 * @param refFile
 * @param objFile
 * @param outFile
 */
void mainSphereTest(char *refFile, char *objFile, char *outFile) {

	int wcsext = 2;
	int magErrThreshold = 0.05; //used by getMagDiff

	StarFileFits *refStarFile, *objStarFile, *refnStarFile, *objnStarFile;
	refStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	objStarFile = new StarFileFits(objFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	refnStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	objnStarFile = new StarFileFits(objFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	refStarFile->readStar(true);
	objStarFile->readStar(false);
	refnStarFile->readStar(true);
	objnStarFile->readStar(false);

	CrossMatchSphere *cms = new CrossMatchSphere();
	printf("sphere match\n");
	cms->match(refStarFile, objStarFile, areaBox);
	printf("sphere match NoPartition\n");
	cms->matchNoPartition(refnStarFile, objnStarFile, areaBox);
	printf("sphere compare\n");
	cms->compareResult(objStarFile, objnStarFile, "out_sphere.cat", areaBox);

	delete objStarFile;
	delete refStarFile;
	delete objnStarFile;
	delete refnStarFile;
}

/*void mainPlaneTest(char *refFile, char *objFile, char *outFile) {

	int wcsext = 2;
	int magErrThreshold = 0.05; //used by getMagDiff

	StarFileFits *refStarFile, *objStarFile, *refnStarFile, *objnStarFile;
	refStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	objStarFile = new StarFileFits(objFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	refnStarFile = new StarFileFits(refFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	objnStarFile = new StarFileFits(objFile, areaBox, fitsHDU, wcsext,
			fluxRatioSDTimes, magErrThreshold, gridX, gridY);
	refStarFile->readStar(true);
	objStarFile->readStar(false);
	refnStarFile->readStar(true);
	objnStarFile->readStar(false);

	CrossMatch *cms = new CrossMatch();
	printf("plane match\n");
	cms->match(refStarFile, objStarFile, areaBox);
	printf("plane match NoPartition\n");
	cms->matchNoPartition(refnStarFile, objnStarFile, areaBox);
	printf("plane compare\n");
	cms->compareResult(objStarFile, objnStarFile, "out_plane.cat", areaBox);

	delete objStarFile;
	delete refStarFile;
	delete objnStarFile;
	delete refnStarFile;

}*/
