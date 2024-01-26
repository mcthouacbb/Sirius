#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <functional>

namespace search
{

constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

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

SEARCH_PARAM(hardTimeScale, 50, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 60, 30, 100, 5);
SEARCH_PARAM(incrementScale, 75, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 135, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 150, 100, 200, 5);

SEARCH_PARAM(maxHistBonus, 1896, 1024, 3072, 256);
SEARCH_PARAM(histScaleQuadratic, 4, 1, 8, 1);
SEARCH_PARAM(histScaleLinear, 120, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 120, 64, 768, 64);

SEARCH_PARAM(aspInitDelta, 15, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 5, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 8, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImprovingMargin, 50, 30, 80, 5);
SEARCH_PARAM(rfpMargin, 80, 50, 100, 5);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpBaseReduction, 3, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 3, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 200, 50, 300, 25);
SEARCH_PARAM(nmpMaxEvalReduction, 3, 2, 5, 1);

SEARCH_PARAM(fpBaseMargin, 120, 60, 360, 15);
SEARCH_PARAM(fpDepthMargin, 75, 10, 180, 10);
SEARCH_PARAM(fpMaxDepth, 6, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 8, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 3, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 8, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -100, -120, -30, 10);
SEARCH_PARAM(seePruneMarginQuiet, -60, -120, -30, 10);

SEARCH_PARAM(maxHistPruningDepth, 4, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1536, 512, 4096, 128);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 5, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 4, 2, 12, 2);

SEARCH_PARAM_CALLBACK(lmrBase, 77, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 236, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrHistDivisor, 8192, 4096, 16384, 512);

SEARCH_PARAM(seMinDepth, 9, 4, 12, 1);
SEARCH_PARAM(seTTDepthMargin, 2, 0, 6, 1);
SEARCH_PARAM(seBetaDepthScale, 40, 8, 80, 6);


}
