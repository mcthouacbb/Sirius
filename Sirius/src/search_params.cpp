#include "search_params.h"

#ifdef EXTERNAL_TUNE
#include "comm/uci.h"
#endif

#include <cmath>
#include <array>

namespace search
{

#ifdef EXTERNAL_TUNE

// used to make searchParams initialized after
std::deque<SearchParam>& searchParams()
{
    static std::deque<SearchParam> params;
    return params;
}

SearchParam& addSearchParam(std::string name, int value, int min, int max, int step, std::function<void()> callback)
{
    searchParams().push_back({name, value, value, min, max, step, callback});
    SearchParam& param = searchParams().back();
    return param;
}

void printWeatherFactoryConfig()
{
    std::cout << "{\n";
    for (auto& param : searchParams())
    {
        std::cout << "    \"" << param.name << "\": {\n";
        std::cout << "        \"value\": " << param.defaultValue << ",\n";
        std::cout << "        \"min_value\": " << param.min << ",\n";
        std::cout << "        \"max_value\": " << param.max << ",\n";
        std::cout << "        \"step\": " << param.step << "\n";
        std::cout << "    }";
        // terrible
        if (&param != &searchParams().back())
            std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "}";
}

void printOpenBenchConfig()
{
    for (auto& param : searchParams())
    {
        std::cout << param.name << " int "
            << param.defaultValue << ' '
            << param.min << ' '
            << param.max << ' '
            << param.step << ' '
            << "0.002" << std::endl;
    }
}

#endif

extern std::array<std::array<int, 64>, 64> lmrTable;

void updateLmrTable()
{
    for (int d = 1; d < 64; d++)
    {
        for (int i = 1; i < 64; i++)
        {
            lmrTable[d][i] = static_cast<int>(lmrBase / 100.0 + std::log(static_cast<double>(d)) * std::log(static_cast<double>(i)) / (lmrDivisor / 100.0));
        }
    }
}


}