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
SEARCH_PARAM(softTimeScale, 62, 30, 100, 5);
SEARCH_PARAM(incrementScale, 88, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 21, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 144, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 165, 100, 200, 5);

SEARCH_PARAM(maxHistBonus, 2136, 1024, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 8, 1, 8, 1);
SEARCH_PARAM(histBonusLinear, 204, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 147, 64, 768, 64);

SEARCH_PARAM(maxHistMalus, 1039, 1024, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 5, 1, 8, 1);
SEARCH_PARAM(histMalusLinear, 243, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 66, 64, 768, 64);

SEARCH_PARAM(histBetaMargin, 50, 30, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 8192, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 2048, 1024, 8192, 64);

SEARCH_PARAM(pawnCorrWeight, 295, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 316, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 277, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 280, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 306, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 322, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 256, 96, 768, 64);

SEARCH_PARAM(aspInitDelta, 9, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 5, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 5, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 32, 30, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 87, 50, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 408, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 455, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalBaseMargin, 171, 50, 300, 10);
SEARCH_PARAM(nmpEvalDepthMargin, 23, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 4, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 210, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 200, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(fpBaseMargin, 160, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 124, 10, 180, 12);
SEARCH_PARAM(fpMaxDepth, 4, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 10, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 7, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -99, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -64, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 98, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 32, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 5, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1768, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 7, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 15, 8, 32, 1);
SEARCH_PARAM(maxMultiExts, 6, 3, 12, 1);
SEARCH_PARAM(doubleExtMargin, 14, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);
SEARCH_PARAM(lmrCorrplexityMargin, 80, 40, 120, 5);

SEARCH_PARAM_CALLBACK(lmrBase, 71, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 224, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 9124, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6036, 2048, 16384, 512);

SEARCH_PARAM(doDeeperMarginBase, 35, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 32, 8, 96, 5);

SEARCH_PARAM(doShallowerMargin, 8, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 68, 0, 250, 16);


}
