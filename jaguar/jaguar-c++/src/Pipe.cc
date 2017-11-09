#include <Jaguar/Pipe.h>

namespace Jaguar {
    Pipe::Pipe(const char* pipeDir) {
        BOOST_LOG_TRIVIAL(info) << "连接管道......\n";
        inputPipe.open(pipeDir);
        BOOST_LOG_TRIVIAL(info) << "连接管道成功！\n";
    }

    Pipe::~Pipe() {
        inputPipe.close();
    }

    void Pipe::getAndInform(Cache* cc) {
        while (true) {
            getline(this->inputPipe, this->lineBuffer);

            if (this->lineBuffer == "start") {
                BOOST_LOG_TRIVIAL(info) << "新批次数据来临\n";
                this->status = PIPE_START;
                continue;
            }

            if (this->lineBuffer == "end") {
                BOOST_LOG_TRIVIAL(info) << "当前批次数据写入完毕\n";
                this->status = PIPE_END;
                this->_Cond.notify_all();
                // 这里需要添加代码，等所有结果收集完毕后，再开始读取下一批数据
                for (int i = 1; i <= cc->getAlgoNum(); i++) {
                    cc->waitSignal();
                }
                break;
            }

            if (this->lineBuffer.length() < 3) {
                BOOST_LOG_TRIVIAL(warning) << "行数据不完整\n";
                this->status = PIPE_ERROR;
                continue;
            }

            if (this->status == PIPE_START) {
                BOOST_LOG_TRIVIAL(info) << "时间戳：" << this->lineBuffer << "\n";
                this->status = PIPE_TIMESTAMP;
                continue;
            }

            int starNum;
            float value;
            long long timeStamp;
            std::vector<std::string> fields;
            boost::split(fields, this->lineBuffer, boost::is_any_of(" "));
            
            this->converter.clear();
            this->converter.str("");
            this->converter << fields[0];
            this->converter >> starNum;
            
            this->converter.clear();
            this->converter.str("");
            this->converter << fields[1];
            this->converter >> value;
            
            this->converter.clear();
            this->converter.str("");
            this->converter << fields[2];
            this->converter >> timeStamp;

            cc->setData(starNum, value, timeStamp);
            this->status = PIPE_DATA;
            continue;
        }
    }
}