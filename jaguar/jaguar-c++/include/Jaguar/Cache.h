#ifndef _JAGUAR_CACHE_H_
#define _JAGUAR_CACHE_H_

#include <sstream>
#include <vector>
#include <unistd.h>

#include <boost/log/trivial.hpp>

#include <Jaguar/Utility.h>
#include <Jaguar/BlockingQueue.h>

namespace Jaguar {

	using std::vector;
	using std::stringstream;

	class Cache {
		friend class App;

	public:
		Cache(int, int);
		~Cache();

		int getObjectNum() { return this->objectNum; }
		int getLen() { return this->len; }
		long long getNewestTime() { return this->newestTime; }
		long long getDataTime(int);

		void addAlgoNum() { this->algoNum++; }
		int getAlgoNum() { return this->algoNum; }

		float getLastOne(int);
		vector<float> getLastMany(int, int);
		ResultWrapper getResult(int algoNum, int StarNum) { return resultBlob[algoNum - 1][StarNum - 1]; }

		void setData(int, float, long long);
		void setResult(int, int, ResultWrapper);

		void addSignal();
		void waitSignal();
		
	private:
		stringstream _Converter;

		int algoNum;
		int objectNum;
		int len;
		int capacity;
		long long newestTime;

		vector<vector<Data>> dataBlob;
		vector<vector<ResultWrapper>> resultBlob;
		BlockingQueue<int> signalQueue;
	};
} //namespace Jaguar

#endif
