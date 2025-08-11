#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// https://stackoverflow.com/a/236803
inline std::vector<std::string> splitByChar(const std::string_view& str, char c)
{
    std::istringstream iss{std::string(str)};
    std::string token;
    std::vector<std::string> result;
    while (std::getline(iss, token, c))
    {
        result.push_back(token);
    }
    return result;
}

inline std::vector<std::string> splitBySpaces(const std::string_view& str)
{
    return splitByChar(str, ' ');
}
