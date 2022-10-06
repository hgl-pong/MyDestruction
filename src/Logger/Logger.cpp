#define _CRT_SECURE_NO_WARNINGS
#include "Logger.h"
#include <windows.h>
#include <iostream>
#include <stdio.h>

#include <stdarg.h>
#include <time.h>

void Logger::ColorPrintOnConsole(DebugType level)
{
        switch(level)
        {
        case DebugType::BLANK://空白
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|
                                FOREGROUND_INTENSITY);
                break;  
        case DebugType::TIME://时间（白色）
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE |
                                BACKGROUND_INTENSITY);
                break;
        case DebugType::INFO://信息（白色蓝底）
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                        FOREGROUND_BLUE|
                        FOREGROUND_INTENSITY);
                break;
        case DebugType::DEBUG://提示（绿色）
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                        FOREGROUND_GREEN |
                        FOREGROUND_INTENSITY);
                break;
        case DebugType::WARN://警告（黄色）
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                        FOREGROUND_GREEN | FOREGROUND_RED |
                        FOREGROUND_INTENSITY);
                break;
        case DebugType::ERR://错误（红色）
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                        FOREGROUND_RED |
                        FOREGROUND_INTENSITY);
                break;
        }
}

std::string Logger::getTime(){

    time_t timep;
    time (&timep); //获取time_t类型的当前时间
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );//对日期和时间进行格式化
    return tmp;
}

void Logger::Debug(std::string moduleName, const char* format, ...){
#ifdef USE_LOGGER
    ColorPrintOnConsole(DebugType::TIME);
    std::cout<<"["<< getTime()<<"]";
    ColorPrintOnConsole(DebugType::DEBUG);
    std::cout<<std::ends<<"Debug"<<std::ends;
	ColorPrintOnConsole(DebugType::INFO);
	std::cout << moduleName << std::ends;
    ColorPrintOnConsole(DebugType::BLANK);
	va_list vp;
	va_start(vp, format);
	vprintf(format, vp);
	va_end(vp);
    printf("\n");
#endif // USE_LOGGER    
}

void Logger::Warning(std::string moduleName, const char* format, ...){
#ifdef USE_LOGGER    
    ColorPrintOnConsole(DebugType::TIME);
    std::cout<<"["<< getTime() <<"]";
    ColorPrintOnConsole(DebugType::WARN);
    std::cout<<std::ends<<"Warning"<<std::ends;
	ColorPrintOnConsole(DebugType::INFO);
	std::cout << moduleName << std::ends;
    ColorPrintOnConsole(DebugType::BLANK);
	va_list vp;
	va_start(vp, format);
	vprintf(format, vp);
	va_end(vp);
    printf("\n");
#endif // USE_LOGGER    
}

void Logger::Error(std::string moduleName, const char* format, ...){
#ifdef USE_LOGGER    
    ColorPrintOnConsole(DebugType::TIME);
    std::cout<<"["<< getTime() <<"]";
    ColorPrintOnConsole(DebugType::ERR);
    std::cout<<std::ends<<"Error"<<std::ends;
	ColorPrintOnConsole(DebugType::INFO);
	std::cout << moduleName << std::ends;
    ColorPrintOnConsole(DebugType::BLANK);
	va_list vp;
	va_start(vp, format);
	vprintf(format, vp);
	va_end(vp);
	printf("\n");
#endif // USE_LOGGER    
}

