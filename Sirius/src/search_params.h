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

SEARCH_PARAM(hardTimeScale, 62, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 57, 30, 100, 5);
SEARCH_PARAM(incrementScale, 83, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 145, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 167, 100, 200, 5);

SEARCH_PARAM(maxHistBonus, 2003, 1024, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 6, 1, 8, 1);
SEARCH_PARAM(histBonusLinear, 167, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 162, 64, 768, 64);

SEARCH_PARAM(maxHistMalus, 1311, 1024, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 4, 1, 8, 1);
SEARCH_PARAM(histMalusLinear, 200, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 68, 64, 768, 64);

SEARCH_PARAM(aspInitDelta, 8, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 2, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 7, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 1, 1, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 12, 4, 10, 1);
SEARCH_PARAM(rfpImprovingMargin, 10, 30, 80, 8);
SEARCH_PARAM(rfpMargin, 20, 50, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 205, 256, 512, 16);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 3, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 208, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 8, 2, 5, 1);

SEARCH_PARAM(fpBaseMargin, 80, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 50, 10, 180, 12);
SEARCH_PARAM(fpMaxDepth, 8, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 10, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 1, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 10, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -50, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -33, -120, -30, 6);

SEARCH_PARAM(maxHistPruningDepth, 5, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 500, 512, 4096, 128);

SEARCH_PARAM(lmrMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 1, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 1, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 134, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 112, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrHistDivisor, 4367, 4096, 16384, 512);

SEARCH_PARAM(qsFpMargin, 30, 0, 250, 16);


}
