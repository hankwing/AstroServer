/* 
 * File:   Partition.h
 * Author: xy
 *
 * Created on October 18, 2013, 8:47 AM
 */

#ifndef PARTITION_H
#define	PARTITION_H

#include "cmhead.h"
#include "StarFile.h"
#include <fstream>

class Partition {
protected:
    float maxx;
    float maxy;
    float minx;
    float miny;
    float fieldWidth; //星表视场的宽度，如果使用时不设置，则程序会自动计算，maxX-minX
    float fieldHeight; //星表视场的高度，如果使用时不设置，则程序会自动计算，maxY-minY

    float errRadius; //两颗星匹配的最小距离
    float searchRadius; //搜索匹配分区时的矩形搜索区域（边长为2*searchRadius）
    float minZoneLength; //最小分区长度 = 3*errRadius
    float zoneInterval; //实际分区长度
    float zoneIntervalRecp; //实际分区长度

    int zoneXnum; //分区在X方向上的个数
    int zoneYnum; //分区在Y方向上的个数
    int totalZone; //分区的总个数
    int totalStar; //星的总数
    CMZone *zoneArray; //分区数组

public:
    Partition();
    Partition(const Partition& orig);
    Partition(float errBox, float minZoneLen, float searchRds);
    virtual ~Partition();

    void partitonStarField(StarFile *starFile);
    std::pair<int, acl::string> getMatchStar(CMStar *objStar, int outputFile);
    void printZoneDetail(char *fName);
    void freeZoneArray();

    void setSearchRadius(float searchRadius);
    float getSearchRadius() const;
    void setErrRadius(float errRadius);
    float getErrRadius() const;
    void setMinZoneLength(float minZoneLength);
    float getMinZoneLength() const;
    void setFieldHeight(float fieldHeight);
    void setFieldWidth(float fieldWidth);
    float getFieldHeight() const;
    float getFieldWidth() const;

protected:
    CMStar *searchSimilarStar(long zoneIdx, CMStar *star);
    long *getStarSearchZone(CMStar *star, long &sZoneNum);
    long getZoneIndex(CMStar * star);
    void getMinMaxXY(CMStar *starList);
    void addStarToZone(CMStar *star, long zoneIdx);
    void freeStarList(CMStar *starList);
};

#endif	/* PARTITION_H */

