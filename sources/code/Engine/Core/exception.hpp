#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>

enum PrintType {
    PRINT_NOTIFICATION = 0,
    PRINT_WARNING,
    PRINT_ERROR,
    PRINT_FATAL_ERROR
};

void Print(PrintType type, std::string str, ...);

#define G_LINED(type, str, ...)             Print(type, std::string(__FILE__) + " (Line " + std::to_string(__LINE__) + "):\n" + str, ##__VA_ARGS__)

#define G_NOTIFY(str, ...)  Print(PRINT_NOTIFICATION, str, ##__VA_ARGS__)
#define G_WARNING(str, ...) G_LINED(PRINT_WARNING, str, ##__VA_ARGS__)
#define G_ERROR(str, ...)   G_LINED(PRINT_ERROR, str, ##__VA_ARGS__)
#define G_FATAL(str, ...)   G_LINED(PRINT_FATAL_ERROR, str, ##__VA_ARGS__)

#endif