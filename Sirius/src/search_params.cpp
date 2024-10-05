#include "search_params.h"

#ifdef EXTERNAL_TUNE
#include "comm/uci.h"
#endif

#include <cmath>
#include "util/multi_array.h"

namespace search
{

MultiArray<int, 64, 64> genLMRTable();

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
        std::cout << param.name << ", int, "
            << param.defaultValue << ", "
            << param.min << ", "
            << param.max << ", "
            << param.step << ", "
            << "0.002" << std::endl;
    }
}

#endif

extern MultiArray<int, 64, 64> lmrTable;

void updateLmrTable()
{
    lmrTable = genLMRTable();
}


}