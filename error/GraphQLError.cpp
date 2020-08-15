#include "GraphQLError.hpp"

GraphQLError::GraphQLError(
    std::string message, std::vector<SourceLocation> *locations = nullptr,
    Source *source = nullptr, std::vector<int> *positions = nullptr,
    std::vector<std::variant<std::string, int>> *path = nullptr,
    std::exception *originalError = nullptr,
    std::map<std::string, std::any> *extensions = nullptr) {
    this->message = message;
    this->locations = locations;
    this->source = source;
    this->positions = positions;
    this->path = path;
    this->originalError = originalError;
    this->extensions = extensions;
}

const char *GraphQLError::what() const throw() { return this->message.c_str(); }