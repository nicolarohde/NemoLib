
#pragma once
#ifndef __NEMOLIB_LOGGER_HPP
#define __NEMOLIB_LOGGER_HPP

#include <mutex>
#include <iostream>

static std::mutex mtx_cout;

// Asynchronous output
struct Logger
{
    std::unique_lock<std::mutex> lk;
    Logger() : lk(std::unique_lock<std::mutex>(mtx_cout)) {}

    template<typename T>
    Logger& operator<<(const T& _t)
    {
        std::cerr << _t;
        return *this;
    }

    Logger& operator<<(std::ostream& (*fp)(std::ostream&))
    {
        std::cerr << fp;
        return *this;
    }
};

#endif
