#include <string>

struct Location
{
    int line;
    int column;
};

class Source
{
public:
    std::string body;
    std::string name;
    Location locationOffset;
    Source(std::string body, std::string name, Location locationOffset);
    Source();
};