#include <Jaguar/Cache.h>

namespace Jaguar {
	Cache::Cache(char* addr, int port, int connRetryTime) {
		this->_RedisContext = redisConnect(addr, port);
		int retryTime = 0;
		while (this->_RedisContext == NULL || this->_RedisContext->err) {
			if (retryTime >= connRetryTime) {
				BOOST_LOG_TRIVIAL(fatal) << "Cache构造失败，程序终止\n";
				exit(1);
			}
			retryTime++;
			BOOST_LOG_TRIVIAL(error) << "Cache构造失败，第" << retryTime << "次尝试重新连接......\n";
			sleep(1000);
			this->_RedisContext = redisConnect(addr, port);
		}
	}

	Cache::~Cache() {
		redisfree(this->_RedisContext);
		BOOST_LOG_TRIVIAL(info) << "释放Cache......\n";
	}

	float Cache::getLastOne(int starNum) {
		redisReply *reply;
		reply = (redisReply*)redisCommand(this->_RedisContext, "LRANGE %d -1 -1", starNum);
		this->_Converter << reply->element[0]->str;
		float result;
		this->_Converter >> result;
		this->_Converter.sync();
		freeReplyObject(reply);
		return result;
	}

	bool Cache::set(Data* d) {
		redisReply *reply;
		reply = (redisReply*)redisCommand(this->_RedisContext, "RPUSH %d %s", d->starNum, d->valueAndTimeStamp);
		if (reply->type != REDIS_REPLY_ERROR) {
			freeReplyObject(reply);
			return true;
		}
		freeReplyObject(reply);
		return false;
	}


} // namespace Jaguar