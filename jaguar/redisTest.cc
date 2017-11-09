#include <boost/log/trivial.hpp>
#include <hiredis/hiredis.h>
#include <vector>

using namespace std;

int main(int, char*[])
{
    redisContext *c = redisConnect("127.0.0.1", 6666);
    if (c == NULL || c->err) {
        if (c) {
            BOOST_LOG_TRIVIAL(error) << "Error: "  << c->errstr << "\n";
        } else {
            BOOST_LOG_TRIVIAL(error) << "Can't allocate redis context\n";
        }
    }
    BOOST_LOG_TRIVIAL(info) << "Successfully connected to 127.0.0.1:6666\n";
    
    //redisReply *reply = (redisReply*)redisCommand(c, "RPUSH %d %s ", 1, "9.999:212121");
    //BOOST_LOG_TRIVIAL(info) << reply->integer << "\n";
    //freeReplyObject(reply);

    redisReply* reply = (redisReply*)redisCommand(c, "LRANGE 1 0 -1");
    BOOST_LOG_TRIVIAL(info) << reply->type << "\n";
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < reply->elements; i++) {
            BOOST_LOG_TRIVIAL(info) << reply->element[i]->str << "\n";
        }
    }
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(c, "LRANGE 1 -1 -1");
    BOOST_LOG_TRIVIAL(info) << reply->type << "\n";
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < reply->elements; i++) {
            BOOST_LOG_TRIVIAL(info) << reply->element[i]->str << "\n";
        }
    }
    freeReplyObject(reply);

    //free the redis context
    redisFree(c);
    return 0;
}
