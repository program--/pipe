#include "pipe.hpp"

#include <iostream>
#include <string>

int print_string(const std::string& msg)
{
    std::cout << msg << std::endl;
    return 1;
}

int main() {
    pipe::with(3)
    | [](double v) -> double { return v + 10; }
    | [](double v) -> char { return static_cast<char>(v); }
    | [](char c)   -> std::string { return std::to_string(c); }
    | [](const std::string& msg) { return print_string(msg); }
    | pipe::done;
}
