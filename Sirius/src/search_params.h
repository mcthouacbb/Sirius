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

SEARCH_PARAM(aspInitDelta, 12, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 5, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 7, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 2, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImprovingMargin, 40, 30, 80, 8);
SEARCH_PARAM(rfpMargin, 86, 50, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 410, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 450, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalBaseMargin, 175, 50, 300, 10);
SEARCH_PARAM(nmpEvalDepthMargin, 25, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 4, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 208, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(fpBaseMargin, 160, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 124, 10, 180, 12);
SEARCH_PARAM(fpMaxDepth, 4, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 10, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 7, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -99, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -66, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 100, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 32, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 5, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1729, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 7, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 16, 8, 32, 1);
SEARCH_PARAM(maxMultiExts, 6, 3, 12, 1);
SEARCH_PARAM(doubleExtMargin, 15, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 65, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 224, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 8735, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6144, 2048, 16384, 512);

SEARCH_PARAM(qsFpMargin, 60, 0, 250, 16);


}
