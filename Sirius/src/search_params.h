#pragma once

#include <cstdint>
#include <string>
#include <deque>
#include <functional>

#define EXTERNAL_TUNE

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

SEARCH_PARAM(hardTimeScale, 60, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 69, 30, 100, 5);
SEARCH_PARAM(incrementScale, 92, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 142, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 163, 100, 200, 5);

SEARCH_PARAM(bmStabilityBase, 67, 20, 200, 10);
SEARCH_PARAM(bmStabilityMin, 89, 60, 120, 15);
SEARCH_PARAM(bmStabilityScale, 944, 200, 1600, 40);
SEARCH_PARAM(bmStabilityOffset, 283, 100, 1000, 40);
SEARCH_PARAM(bmStabilityPower, -157, -250, -120, 8);

SEARCH_PARAM(maxHistBonus, 2076, 1024, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 8, 1, 8, 1);
SEARCH_PARAM(histBonusLinear, 213, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 156, 64, 768, 64);

SEARCH_PARAM(maxHistMalus, 1025, 1024, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 5, 1, 8, 1);
SEARCH_PARAM(histMalusLinear, 247, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 65, 64, 768, 64);

SEARCH_PARAM(histBetaMargin, 46, 30, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 8249, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 1948, 1024, 8192, 64);

SEARCH_PARAM(pawnCorrWeight, 356, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 348, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 250, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 309, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 309, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 354, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 230, 96, 768, 64);
SEARCH_PARAM(contCorr3Weight, 231, 96, 768, 64);
SEARCH_PARAM(contCorr4Weight, 201, 96, 768, 64);
SEARCH_PARAM(contCorr5Weight, 193, 96, 768, 64);
SEARCH_PARAM(contCorr6Weight, 183, 96, 768, 64);
SEARCH_PARAM(contCorr7Weight, 164, 96, 768, 64);

SEARCH_PARAM(aspInitDelta, 8, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 6, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 4, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 31, 30, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 87, 50, 100, 8);
SEARCH_PARAM(rfpOppWorsening, 14, 10, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 410, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 453, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalBaseMargin, 176, 50, 300, 10);
SEARCH_PARAM(nmpEvalDepthMargin, 24, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 4, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 213, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 201, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(fpBaseMargin, 151, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 120, 10, 180, 12);
SEARCH_PARAM(fpMaxDepth, 4, 4, 9, 1);

SEARCH_PARAM(lmpMaxDepth, 10, 4, 11, 1);
SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(maxSeePruneDepth, 7, 6, 11, 1);
SEARCH_PARAM(seePruneMarginNoisy, -94, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -60, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 96, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 31, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 7, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1787, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 6, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 14, 8, 32, 1);
SEARCH_PARAM(maxMultiExts, 6, 3, 12, 1);
SEARCH_PARAM(doubleExtMargin, 12, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);
SEARCH_PARAM(lmrCorrplexityMargin, 85, 40, 120, 5);

SEARCH_PARAM_CALLBACK(lmrBase, 73, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 224, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 8908, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 5828, 2048, 16384, 512);

SEARCH_PARAM(doDeeperMarginBase, 35, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 35, 8, 96, 5);

SEARCH_PARAM(doShallowerMargin, 8, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 57, 0, 250, 16);


}
