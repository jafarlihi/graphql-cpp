#include <any>
#include <exception>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "../language/location.hpp"
#include "../language/source.hpp"

class GraphQLError : public std::exception {
    std::string message;
    std::vector<SourceLocation> *locations;
    Source *source;
    std::vector<int> *positions;
    std::vector<std::variant<std::string, int>> *path;
    std::exception *originalError;
    std::map<std::string, std::any> *extensions;

    GraphQLError(std::string message,
                 std::vector<SourceLocation> *locations = nullptr,
                 Source *source = nullptr,
                 std::vector<int> *positions = nullptr,
                 std::vector<std::variant<std::string, int>> *path = nullptr,
                 std::exception *originalError = nullptr,
                 std::map<std::string, std::any> *extensions = nullptr);
    virtual const char *what() const throw();
};