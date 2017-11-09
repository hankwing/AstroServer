#ifndef _JAGUAR_ALGORITHM_H_
#define _JAGUAR_ALGORITHM_H_

#include <functional>
#include <condition_variable>
#include <mutex>
#include <algorithm>

#include <boost/log/trivial.hpp>

#include <Jaguar/Utility.h>
#include <Jaguar/Cache.h>

namespace Jaguar {

    typedef Result AlgoFunc(int, Cache*);
    
    class Algorithm {
        friend class App;
    public:
        Algorithm() = delete;
        Algorithm(int _Num, const char* _Name, AlgoFunc _Algo,  std::condition_variable* _CP, std::mutex* _MP);
        const char* getName() { return AlgoName; }
        void threadFunc(Cache*);
    private:
        int num;
        const char* AlgoName;
        std::function<Result(int, Cache*)> AlgoImpl;
        std::condition_variable* CondPtr;
        std::mutex* MtxPtr;
    };
}

#endif