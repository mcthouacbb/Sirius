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

SEARCH_PARAM(hardTimeScale, 45, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 61, 30, 100, 5);
SEARCH_PARAM(incrementScale, 78, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 19, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 143, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 152, 100, 200, 5);

SEARCH_PARAM(maxHistBonus, 1945, 1024, 3072, 128);
SEARCH_PARAM(histScaleQuadratic, 5, 1, 8, 1);
SEARCH_PARAM(histScaleLinear, 155, 64, 384, 16);
SEARCH_PARAM(histBonusOffset, 181, 64, 768, 32);

SEARCH_PARAM(aspInitDelta, 15, 8, 30, 1);
SEARCH_PARAM(minAspDepth, 4, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 7, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 2, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 6, 4, 10, 1);
SEARCH_PARAM(rfpImprovingMargin, 48, 30, 80, 3);
SEARCH_PARAM(rfpMargin, 79, 50, 100, 3);
SEARCH_PARAM(rfpHistDivisor, 381, 256, 512, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpBaseReduction, 4, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 3, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 197, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 3, 2, 5, 1);

SEARCH_PARAM(fpBaseMargin, 114, 60, 360, 8);
SEARCH_PARAM(fpDepthMargin, 74, 10, 180, 8);
SEARCH_PARAM(fpMaxDepth, 6, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 8, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 9, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -107, -120, -30, 5);
SEARCH_PARAM(seePruneMarginQuiet, -63, -120, -30, 5);

SEARCH_PARAM(maxHistPruningDepth, 6, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1546, 512, 4096, 128);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 2, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 5, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 4, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 88, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 224, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrHistDivisor, 7844, 4096, 16384, 512);


}
