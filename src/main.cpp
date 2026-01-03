#include "Application.h"
#include <iostream>

int main()
{
    // 实例化 Part A 的核心应用对象
    Application app("Graphics Engine Group Project", 1280, 720);

    try
    {
        // 启动引擎
        app.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Application Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}