all: main

main: src/main.cc src/Algorithm.cc src/Cache.cc src/Pipe.cc src/Jaguar.cc
	mkdir bin
	g++ -std=c++11 -o bin/main src/main.cc src/Algorithm.cc src/Cache.cc src/Pipe.cc src/Jaguar.cc -g -I./include/ -DBOOST_LOG_DYN_LINK -lboost_log -lpthread 

clean: 
	rm -rf bin 
