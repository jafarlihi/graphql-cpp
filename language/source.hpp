#include <string>

struct Location
{
    int line;
    int column;
};

class Source
{
private:
    std::string body;
    std::string name;
    Location locationOffset;

public:
    Source(std::string body, std::string name, Location locationOffset);
};