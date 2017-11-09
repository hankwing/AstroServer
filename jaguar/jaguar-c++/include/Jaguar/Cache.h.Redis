#ifndef _JAGUAR_CACHE_H_
#define _JAGUAR_CACHE_H_

#include <sstream>

#include <unistd.h>

#include <hiredis/hiredis.h>
#include <boost/log/trivial.hpp>

#include <Jaguar/Datatype.h>

namespace Jaguar {
	class Cache {
		friend class App;
	public:
		Cache(char* addr = "127.0.0.1", int port = 6666, int connRetryTime = 3);
		~Cache();
		float getLastOne(int);
		float getLastMany(int, int);
	private:
		bool set(Data);
		redisContext* _RedisContext;
		std::stringstream _Converter;
	};
} //namespace Jaguar

#endif
