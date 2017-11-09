/* 
 * File:   StoreData.h
 * Author: xy
 *
 * Created on 2014年12月13日, 下午8:34
 */

#ifndef STOREDATA_H
#define	STOREDATA_H

#include "StarFile.h"

class StoreData {
protected:

public:
    virtual void store(StarFile *starFile) = 0;
};

#endif	/* STOREDATA_H */

