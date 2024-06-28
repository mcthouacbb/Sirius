#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <functional>

namespace search
{

#ifdef EXTERNAL_TUNE


struct SearchParam
{
    std::string name;
    int value;
    int defaultValue;
    int min;
    int max;
    int step;
    std::function<void()> callback;
};

std::deque<SearchParam>& searchParams();
SearchParam& addSearchParam(std::string name, int value, int min, int max, int step, std::function<void()> callback = std::function<void()>());
void printWeatherFactoryConfig();
void printOpenBenchConfig();
void updateLmrTable();

#define SEARCH_PARAM(name, val, min, max, step) \
    inline SearchParam& name##Param = addSearchParam(#name, val, min, max, step); \
    inline const int& name = name##Param.value
#define SEARCH_PARAM_CALLBACK(name, val, min, max, step, callback) \
    inline SearchParam& name##Param = addSearchParam(#name, val, min, max, step, callback); \
    inline const int& name = name##Param.value
#else
#define SEARCH_PARAM(name, val, min, max, step) constexpr int name = val;
#define SEARCH_PARAM_CALLBACK(name, val, min, max, step, callback) SEARCH_PARAM(name, val, min, max, step)
#endif

SEARCH_PARAM(hardTimeScale, 64, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 59, 30, 100, 5);
SEARCH_PARAM(incrementScale, 85, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 21, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 143, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 165, 100, 200, 5);

SEARCH_PARAM(maxHistBonus, 1907, 1024, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 4, 1, 8, 1);
SEARCH_PARAM(histBonusLinear, 179, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 121, 64, 768, 64);

SEARCH_PARAM(maxHistMalus, 1838, 1024, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 4, 1, 8, 1);
SEARCH_PARAM(histMalusLinear, 199, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 88, 64, 768, 64);

SEARCH_PARAM(aspInitDelta, 20, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 5, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 6, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 3, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 9, 4, 10, 1);
SEARCH_PARAM(rfpImprovingMargin, 49, 30, 80, 8);
SEARCH_PARAM(rfpMargin, 84, 50, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 417, 256, 512, 16);

SEARCH_PARAM(nmpMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 5, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 207, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(fpBaseMargin, 162, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 120, 10, 180, 12);
SEARCH_PARAM(fpMaxDepth, 4, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 10, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 8, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -100, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -64, -120, -30, 6);

SEARCH_PARAM(maxHistPruningDepth, 3, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1670, 512, 4096, 128);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 3, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 61, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 222, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrHistDivisor, 8559, 4096, 16384, 512);

SEARCH_PARAM(qsFpMargin, 75, 0, 250, 16);


}
