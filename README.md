# pipe: an over-engineered forward-pipe C++14 library

## Usage

`pipe::with` describes your initial input value. This can then be piped into objects that provide an `operator()` member, such as lambdas, using the `|` operator. Finally, adding `| pipe::done` signals that the operation is complete, and should be evaluated.

If the piped functors are `constexpr`-able, then the entire pipeline is `constexpr`-able.

### Example

```cpp
// example.cpp
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
```
