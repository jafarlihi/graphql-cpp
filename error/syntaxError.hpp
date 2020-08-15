#include <string>

#include "../language/source.hpp"
#include "GraphQLError.hpp"

GraphQLError *syntaxError(Source source, int position, std::string description);