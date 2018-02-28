#include "exception.hpp"
#include <stdarg.h>
#include <iostream>

void Print(PrintType type, std::string str, ...) {
    va_list args;
    va_start(args, str);
    switch (type) {
    default:
    case PRINT_NOTIFICATION:
        vprintf(str.c_str(), args);
        break;
    case PRINT_WARNING:
        vprintf(("\033[1;33m" + str + "\033[0m").c_str(), args);
        break;
    case PRINT_ERROR:
        vprintf(("\033[1;31m" + str + "\033[0m").c_str(), args);
        break;
    case PRINT_FATAL_ERROR:
        vprintf(("\033[1;35m" + str + "\033[0m").c_str(), args);
        break;
    }
    va_end(args);
}