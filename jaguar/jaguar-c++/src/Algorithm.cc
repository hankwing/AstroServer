#include <Jaguar/Algorithm.h>

namespace Jaguar {
    Algorithm::Algorithm(int _Num, const char* _Name, Result _Algo(int, Cache*), std::condition_variable* _CP, std::mutex* _MP) 
        : num(_Num), AlgoName(_Name), AlgoImpl(_Algo), CondPtr(_CP), MtxPtr(_MP) {        
    }

    void Algorithm::threadFunc(Cache* cc) {
        while (true) {
            std::unique_lock<std::mutex> _lck(*(this->MtxPtr));
            (*(this->CondPtr)).wait(_lck);
            _lck.unlock();
            long long newestTime = cc->getNewestTime();
            for (int i = 1; i <= cc->getObjectNum(); i++) {
                /*
                if (cc->getDataTime(i) != newestTime) {
                    continue;
                }
                */
                Result res = this->AlgoImpl(i, cc);
                cc->setResult(this->num, i, ResultWrapper(res, newestTime));
            }
            cc->addSignal();
        }   
    }
}