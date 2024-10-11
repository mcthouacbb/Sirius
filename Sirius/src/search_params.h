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

SEARCH_PARAM(hardTimeScale, 63, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 63, 30, 100, 5);
SEARCH_PARAM(incrementScale, 88, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 21, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 146, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 166, 100, 200, 5);

SEARCH_PARAM(maxHistBonus, 2046, 1024, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 7, 1, 8, 1);
SEARCH_PARAM(histBonusLinear, 201, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 152, 64, 768, 64);

SEARCH_PARAM(maxHistMalus, 1025, 1024, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 5, 1, 8, 1);
SEARCH_PARAM(histMalusLinear, 231, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 67, 64, 768, 64);

SEARCH_PARAM(aspInitDelta, 9, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 5, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 5, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 2, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImprovingMargin, 35, 30, 80, 8);
SEARCH_PARAM(rfpMargin, 86, 50, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 405, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 452, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalBaseMargin, 171, 50, 300, 10);
SEARCH_PARAM(nmpEvalDepthMargin, 23, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 4, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 210, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(fpBaseMargin, 161, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 126, 10, 180, 12);
SEARCH_PARAM(fpMaxDepth, 4, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 10, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 7, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -100, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -64, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 98, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 32, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 5, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1753, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 7, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 15, 8, 32, 1);
SEARCH_PARAM(maxMultiExts, 6, 3, 12, 1);
SEARCH_PARAM(doubleExtMargin, 15, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 67, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 227, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 9170, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6124, 2048, 16384, 512);

SEARCH_PARAM(qsFpMargin, 65, 0, 250, 16);


}
