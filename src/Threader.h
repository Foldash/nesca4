#ifndef THREADER_H
#define THREADER_H
#include "ThreadPool.h"
#include <string>

class Threader {
public:
    static int gThreadDelay;
    static void start();
    static bool fireThread(std::string ip, void (*func)(std::string));
    static void requestStop();
    static void finish();
    static void cleanUp();
    static nesca::ThreadPool::Stats stats();
private:
    static nesca::ThreadPool pool_;
};
#endif
