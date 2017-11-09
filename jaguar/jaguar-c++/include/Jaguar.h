#ifndef _JAGUAR_H_
#define _JAGUAR_H_

#include <vector>
#include <thread>

#include <Jaguar/Pipe.h>
#include <Jaguar/Cache.h>
#include <Jaguar/Algorithm.h>
#include <Jaguar/Utility.h>

namespace Jaguar {
    class App {
    public:
        App() = delete;
        App(const char*, int, int);
        ~App();
        void run();
        void registerAlgorithm(AlgoFunc _Algo, const char* _Name);
    private:
        int num;
        int algoNum;
        Pipe* _Pipe;
        Cache* _Cache;
        std::vector<Algorithm> AlgoSet;
    };
} // namespace Jaguar

#endif

