#include "devAssert.hpp"

void devAssert(bool condition, std::string message) {
    if (!condition) throw std::runtime_error(message);
}