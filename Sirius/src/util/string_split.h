#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// https://stackoverflow.com/a/236803
std::vector<std::string> splitBySpaces(const std::string_view& str)
{
    std::istringstream iss{std::string(str)};
    std::string token;
    std::vector<std::string> result;
    while (std::getline(iss, token, ' '))
    {
        result.push_back(token);
    }
    return result;
}
