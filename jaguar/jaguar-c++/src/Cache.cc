#include <Jaguar/Cache.h>

namespace Jaguar {
	Cache::Cache(int objectNum_, int len_) : algoNum(0), objectNum(objectNum_), len(len_), capacity(len_ * 2), newestTime(0) {
		for (int i = 0; i < objectNum; i++) {
			this->dataBlob.push_back(vector<Data>());
		}
	}

	Cache::~Cache() {

	}
	
	long long Cache::getDataTime(int starNum) {
		if (starNum > objectNum) {
			BOOST_LOG_TRIVIAL(fatal) << "getDataTime(" << starNum << ")时发生错误：编号超出预设定的范围\n";
			return -1;
		} else if (this->dataBlob[starNum - 1].empty()) {
			BOOST_LOG_TRIVIAL(fatal) << "getDataTime(" << starNum << ")时发生错误：对应编号的观测对象没有数据\n";
		} else {
			return this->dataBlob[starNum - 1].back().timeStamp;
		}
	}

	float Cache::getLastOne(int starNum) {
		if (starNum > objectNum) {
			BOOST_LOG_TRIVIAL(fatal) << "getLastOne(" << starNum << ")时发生错误：编号超出预设定的范围\n";
			return -1.0;
		} else if (dataBlob[starNum - 1].empty()) {
			BOOST_LOG_TRIVIAL(fatal) << "getLastOne(" << starNum << ")时发生错误：对应编号的观测对象没有数据\n";
			return -1.0;
		} else {
			return dataBlob[starNum - 1].back().value;
		}
	}

	vector<float> Cache::getLastMany(int starNum, int len) {
		if (starNum > objectNum) {
			BOOST_LOG_TRIVIAL(fatal) << "getLastMany(" << starNum << ")时发生错误：编号超出预设定的范围\n";
			return vector<float>();
		} else if (dataBlob[starNum - 1].size() < len) {
			BOOST_LOG_TRIVIAL(fatal) << "getLastMany(" << starNum << ")时发生错误：编号超出预设定的范围\n";
			return vector<float>();
		} else {
			vector<float> result;
			for (int i = 0; i < len; i++) {
				result.push_back(dataBlob[starNum - 1][dataBlob[starNum - 1].size() - 1 - i].value);
			}
			return result;
		}
	}

	void Cache::setData(int starNum, float value, long long timeStamp) {
		dataBlob[starNum - 1].push_back(Data(value, timeStamp));
		if (dataBlob[starNum - 1].size() > this->capacity) {
			for (int i = 0; i < this->len; i++) {
				dataBlob[starNum - 1][i] = *(dataBlob[starNum - 1].end() - len + i);
			}
			dataBlob[starNum - 1].resize(len);
		}
		if (timeStamp > this->newestTime) {
			this->newestTime = timeStamp;
		}
	}

	void Cache::setResult(int algoNum, int starNum, ResultWrapper rw) {
		this->resultBlob[algoNum - 1][starNum - 1] = rw;
	}

	void Cache::addSignal() {
		this->signalQueue.push(1);
	}

	void Cache::waitSignal() {
		int dummy;
		this->signalQueue.wait_and_pop(dummy);
	}
} // namespace Jaguar