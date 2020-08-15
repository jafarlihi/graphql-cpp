#include "syntaxError.hpp"

#include "GraphQLError.hpp"

GraphQLError *syntaxError(Source source, int position,
                          std::string description) {
    std::vector<int> positions = {position};
    return new GraphQLError(description);
}