/* 
 * File:   CrossMatch.h
 * Author: xy
 *
 * Created on October 18, 2013, 8:48 AM
 */

#ifndef CROSSMATCH_H
#define	CROSSMATCH_H

#include "StarFile.h"
#include "Partition.h"
#include "PartitionSphere.h"

class CrossMatch {
public:

    static const int TimesOfErrorRadius = 10; //length of search radius
    CrossMatch();
    CrossMatch(const CrossMatch& orig);
    CrossMatch(StarFile* refStarFile, StarFile* objStarFile);
    virtual ~CrossMatch();

    void match(char *refName, char *objName, float errorBox, int outputFile);
    void match(StarFile *ref, StarFile *obj, float errorBox,int outputFile);
    void match(StarFile *ref, StarFile *obj,Partition * zones, float errorBox, int outputFile);
    void compareResult(char *refName, char *objName, char *outName, float errorBox);
    void compareResult(StarFile *objStarFile,StarFile *objStarFileNoPtn, const char *outfName, float errorBox);
    void matchNoPartition(char *refName, char *objName, float errorBox);
    void matchNoPartition(StarFile *ref, StarFile *obj, float errorBox);
    //int printMatchedRstCount();
    void printMatchedRst(char *outfName, float errorBox);
    void printMatchedRst(char *outfName, StarFile *starList, float errorBox);
    void printOTStar(char *outfName, float errorBox);
    //int printOTStarCount();
    void printAllStarList(char *outfName, StarFile *starList, float errorBox);
    void freeAllMemory();
    void testCrossMatch();
    void partitionAndNoPartitionCompare();
    void setFieldHeight(float fieldHeight);
    void setFieldWidth(float fieldWidth);
    void sendResultsToRedis(acl::redis_client_cluster *cluster, StarFileFits *objStars, int usedThreads, int& control);
    //void compressStarData( std::vector<acl::string> data, std::string key);
    //static void *singleSendThread( void * arg);

protected:

    StarFile *refStarFile;
    StarFile *objStarFile;
    StarFile *refStarFileNoPtn;
    StarFile *objStarFileNoPtn;

protected:
    float errRadius; //两颗星匹配的最小距离
    Partition *zones;
    float fieldWidth; //星表视场的宽度，如果使用时不设置，则程序会自动计算，maxX-minX
    float fieldHeight; //星表视场的高度，如果使用时不设置，则程序会自动计算，maxY-minY
};


#endif	/* CROSSMATCH_H */

