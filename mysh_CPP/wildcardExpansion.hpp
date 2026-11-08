#ifndef _MYSH_WILDCARDS
#define _MYSH_WILDCARDS

#include <vector>
#include <string>

namespace mysh{
    std::vector<std::string> expandWildcard(std::string_view pattern);
    bool expandWildcard(std::vector<std::string> *tokens);
}

#endif
