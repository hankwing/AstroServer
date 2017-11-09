#ifndef _JAGUAR_UTILITY_H_
#define _JAGUAR_UTILITY_H_

namespace Jaguar {
    struct Data {
        Data() : value(0.0), timeStamp(0) {}
        Data(float v, long long t) : value(v), timeStamp(t) {}
        float value;
        long long timeStamp;
    };

    struct Result {
        Result() : anomalous(false), anomalyName(NULL) {}
        Result(bool a, const char* aN) : anomalous(a), anomalyName(aN) {}
        bool anomalous;
        const char* anomalyName;
    };

    struct ResultWrapper {
        ResultWrapper() : r(Result()), timeStamp(0) {}
        ResultWrapper(Result _r, long long t) : r(_r), timeStamp(t) {}
        Result r;
        long long timeStamp;
    };

}

#endif