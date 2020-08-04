#include "source.hpp"

#include "../cpputils/devAssert.hpp"

Source::Source(std::string body, std::string name = "GraphQL request",
               Location locationOffset = {.line = 1, .column = 1}) {
    this->body = body;
    this->name = name;
    this->locationOffset = locationOffset;
    devAssert(this->locationOffset.line > 0,
              "Line in locationOffset is 1-indexed and must be positive");
    devAssert(this->locationOffset.column > 0,
              "Column in locationOffset is 1-indexed and must be positive");
}

Source::Source() {}
