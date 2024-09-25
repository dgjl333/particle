#pragma once
#include <iostream>

template<typename T> void print(const T& text)
{
#ifdef _DEBUG
    std::cout << text << std::endl;
#endif
}
