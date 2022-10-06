/*
 * @Author       : HGL
 * @Date         : 2022-06-20 16:55:25
 * @LastEditTime : 2022-06-20 17:02:49
 * @Description  :
 * @FilePath     : \Destrution\Destruction\Base.h
 */
#ifndef BASE
#define BASE
#include <string>
class Application;
class Base
{
public:
    Base(){};
    virtual ~Base(){};
    
public:
    std::string name;
    Application* app;
};
#endif // BASE