#include <Jaguar.h>

namespace Jaguar {
    App::App(const char* pipeDir, int totalStarNum, int cacheLen) : num(totalStarNum), algoNum(0) {
        this->_Pipe = new Pipe(pipeDir);
        this->_Cache = new Cache(totalStarNum, cacheLen);
    }

    App::~App() {
        delete this->_Pipe;
        delete this->_Cache;
    }
    
    void App::registerAlgorithm(AlgoFunc _Algo, const char* _Name) {
        this->_Cache->addAlgoNum();
        this->_Cache->resultBlob.push_back(vector<ResultWrapper>(this->_Cache->getObjectNum(), ResultWrapper()));
        this->algoNum++;
        Algorithm newAlgo(this->algoNum, _Name, _Algo, &this->_Pipe->_Cond, &this->_Pipe->_Mtx);
        this->AlgoSet.push_back(newAlgo);
        std::thread worker(&Algorithm::threadFunc, newAlgo, this->_Cache);
        worker.detach();
    }

    void App::run() {
        if (this->AlgoSet.empty()) {
            BOOST_LOG_TRIVIAL(fatal) << "没有注册任何检测算法!\n";
            exit(1);
        } else {
            BOOST_LOG_TRIVIAL(info) << "已注册的函数:\n";
            for (auto& elem : this->AlgoSet) {
                BOOST_LOG_TRIVIAL(info) << elem.AlgoName << "\n";
            }
        }
        while (true) {
            this->_Pipe->getAndInform(this->_Cache);
            for (int i = 1; i <= this->_Cache->getAlgoNum(); i++) {
                BOOST_LOG_TRIVIAL(info) << this->AlgoSet[i - 1].getName() << "算法(编号为" << i << ")认为存在异常的星体:";
                for (int j = 1; j <= this->_Cache->getObjectNum(); j++) {
                    ResultWrapper rw = this->_Cache->getResult(i, j);
                    if (rw.r.anomalous) {
                        BOOST_LOG_TRIVIAL(info) << j;
                    }
                }
            }
        }
        
    }
} // namespace Jaguar