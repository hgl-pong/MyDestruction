#ifndef LOGGER
#define LOGGER 
#include "../Common/Base.h"
#define _STR(x) _VAL(x)
#define _VAL(x) #x
#define FUNCTION_PATH ((std::string)__FILE__+"#"+_STR(__LINE__)+" line  @"+(std::string)__func__).c_str()
#define PRINT(fmt,...) vprintf(fmt ,##__VA_ARGS__)
#include <string>

class Logger{
    public:
    enum class DebugType{
        BLANK,
        TIME,
        INFO,
        DEBUG,
        WARN,
        ERR
    };
    static void Debug(std::string moduleName,const char* format, ...);
    static void Warning(std::string moduleName,const char* format, ...);
    static void Error(std::string moduleName, const char* format, ...);

    static void ColorPrintOnConsole(DebugType level);
    static std::string getTime();
};
#endif //LOGGER