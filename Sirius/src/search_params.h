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

SEARCH_PARAM(hardTimeScale, 61, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 72, 30, 100, 5);
SEARCH_PARAM(incrementScale, 90, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 142, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 162, 100, 200, 5);

SEARCH_PARAM(bmStabilityBase, 70, 20, 200, 10);
SEARCH_PARAM(bmStabilityMin, 88, 60, 120, 15);
SEARCH_PARAM(bmStabilityScale, 959, 200, 1600, 40);
SEARCH_PARAM(bmStabilityOffset, 283, 100, 1000, 40);
SEARCH_PARAM(bmStabilityPower, -160, -250, -120, 8);

SEARCH_PARAM(maxHistBonus, 2074, 1024, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 8, 1, 8, 1);
SEARCH_PARAM(histBonusLinear, 212, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 159, 64, 768, 64);

SEARCH_PARAM(maxHistMalus, 1024, 1024, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 5, 1, 8, 1);
SEARCH_PARAM(histMalusLinear, 258, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 66, 64, 768, 64);

SEARCH_PARAM(histBetaMargin, 45, 30, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 8329, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 1994, 1024, 8192, 64);

SEARCH_PARAM(pawnCorrWeight, 370, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 347, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 263, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 342, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 312, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 368, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 257, 96, 768, 64);
SEARCH_PARAM(contCorr3Weight, 246, 96, 768, 64);
SEARCH_PARAM(contCorr4Weight, 215, 96, 768, 64);
SEARCH_PARAM(contCorr5Weight, 200, 96, 768, 64);
SEARCH_PARAM(contCorr6Weight, 173, 96, 768, 64);
SEARCH_PARAM(contCorr7Weight, 159, 96, 768, 64);

SEARCH_PARAM(aspInitDelta, 8, 8, 30, 4);
SEARCH_PARAM(minAspDepth, 6, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 3, 1, 32, 2);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 30, 30, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 84, 50, 100, 8);
SEARCH_PARAM(rfpOppWorsening, 15, 10, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 413, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 455, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalBaseMargin, 180, 50, 300, 10);
SEARCH_PARAM(nmpEvalDepthMargin, 24, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 5, 2, 5, 1);
SEARCH_PARAM(nmpDepthReductionScale, 4, 3, 6, 1);
SEARCH_PARAM(nmpEvalReductionScale, 212, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 201, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(fpBaseMargin, 149, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 123, 10, 180, 12);
SEARCH_PARAM(fpHistDivisor, 400, 256, 512, 16);
SEARCH_PARAM(fpMaxDepth, 4, 4, 9, 1);

SEARCH_PARAM(lmpMinMovesBase, 2, 2, 7, 1);

SEARCH_PARAM(seePruneMarginNoisy, -95, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -58, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 97, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 30, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 7, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1743, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 6, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 13, 8, 32, 1);
SEARCH_PARAM(doubleExtMargin, 12, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 1, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 3, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);
SEARCH_PARAM(lmrCorrplexityMargin, 87, 40, 120, 5);

SEARCH_PARAM_CALLBACK(lmrBase, 74, -50, 200, 10, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrDivisor, 228, 180, 320, 10, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 9037, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 5955, 2048, 16384, 512);

SEARCH_PARAM(doDeeperMarginBase, 35, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 36, 8, 96, 5);

SEARCH_PARAM(doShallowerMargin, 8, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 61, 0, 250, 16);


}
