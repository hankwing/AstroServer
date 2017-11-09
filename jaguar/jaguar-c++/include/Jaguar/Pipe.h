#ifndef _JAGUAR_PIPE_H_
#define _JAGUAR_PIPE_H_

#include <condition_variable>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

#include <Jaguar/Utility.h>
#include <Jaguar/Cache.h>

namespace Jaguar {
	enum PipeStatus {
		PIPE_START,
		PIPE_TIMESTAMP,
		PIPE_DATA,
		PIPE_END,
		PIPE_ERROR
	};

	class Pipe {
		friend class App;
	public:
		Pipe() = delete;
		Pipe(const char*);
		~Pipe();
		void getAndInform(Cache*);
		std::ifstream inputPipe;
		std::string lineBuffer;
		std::stringstream converter;
		std::condition_variable _Cond;
		std::mutex _Mtx;
		PipeStatus status;
	};
} //namespace Jaguar

#endif
