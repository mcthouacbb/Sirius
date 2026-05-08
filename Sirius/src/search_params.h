#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <string>

#include "defs.h"

namespace search
{

#ifdef EXTERNAL_TUNE

struct SearchParam
{
    std::string name;
    i32 value;
    i32 defaultValue;
    i32 min;
    i32 max;
    i32 step;
    std::function<void()> callback;
};

std::deque<SearchParam>& searchParams();
SearchParam& addSearchParam(std::string name, i32 value, i32 min, i32 max, i32 step,
    std::function<void()> callback = std::function<void()>());
void printWeatherFactoryConfig();
void printOpenBenchConfig();
void updateLmrTable();

#define SEARCH_PARAM(name, val, min, max, step) \
    inline SearchParam& name##Param = addSearchParam(#name, val, min, max, step); \
    inline const i32& name = name##Param.value
#define SEARCH_PARAM_CALLBACK(name, val, min, max, step, callback) \
    inline SearchParam& name##Param = addSearchParam(#name, val, min, max, step, callback); \
    inline const i32& name = name##Param.value
#else
#define SEARCH_PARAM(name, val, min, max, step) constexpr i32 name = val;
#define SEARCH_PARAM_CALLBACK(name, val, min, max, step, callback) \
    SEARCH_PARAM(name, val, min, max, step)
#endif

SEARCH_PARAM(hardTimeScale, 63, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 68, 30, 100, 5);
SEARCH_PARAM(incrementScale, 87, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 141, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 158, 100, 200, 5);

SEARCH_PARAM(bmStabilityBase, 83, 20, 200, 10);
SEARCH_PARAM(bmStabilityMin, 99, 60, 120, 15);
SEARCH_PARAM(bmStabilityScale, 964, 200, 1600, 40);
SEARCH_PARAM(bmStabilityOffset, 259, 100, 1000, 40);
SEARCH_PARAM(bmStabilityPower, -153, -250, -120, 8);

SEARCH_PARAM(maxHistBonus, 2121, 512, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 439, 1, 1536, 64);
SEARCH_PARAM(histBonusLinear, 196, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 100, -384, 768, 64);

SEARCH_PARAM(maxHistMalus, 992, 512, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 235, 1, 1536, 64);
SEARCH_PARAM(histMalusLinear, 277, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, -44, -384, 768, 64);

SEARCH_PARAM(histBetaMargin, 37, 10, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 8211, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 1969, 1024, 8192, 96);

SEARCH_PARAM(pawnCorrWeight, 384, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 406, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 280, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 252, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 274, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 418, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 362, 96, 768, 64);
SEARCH_PARAM(contCorr3Weight, 146, 96, 768, 64);
SEARCH_PARAM(contCorr4Weight, 194, 96, 768, 64);
SEARCH_PARAM(contCorr5Weight, 176, 96, 768, 64);
SEARCH_PARAM(contCorr6Weight, 166, 96, 768, 64);
SEARCH_PARAM(contCorr7Weight, 127, 96, 768, 64);
SEARCH_PARAM(highCorrplexityMargin, 89, 40, 120, 5);

SEARCH_PARAM(aspInitDelta, 10, 5, 30, 2);
SEARCH_PARAM(minAspDepth, 6, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 46, 1, 512, 24);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 26, 20, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 80, 50, 100, 8);
SEARCH_PARAM(rfpOppEasyCapture, 19, 5, 100, 8);
SEARCH_PARAM(rfpOppWorsening, 14, 5, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 410, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 458, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalMargin, 29, 0, 100, 10);
SEARCH_PARAM(nmpStaticEvalBaseMargin, 193, 50, 300, 10);
SEARCH_PARAM(nmpStaticEvalDepthMargin, 18, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 1343, 512, 2048, 64);
SEARCH_PARAM(nmpDepthReductionScale, 78, 32, 96, 12);
SEARCH_PARAM(nmpEvalReductionScale, 208, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 182, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(mvvPawn, 952, -20000, 20000, 200);
SEARCH_PARAM(mvvKnight, 2467, -20000, 20000, 200);
SEARCH_PARAM(mvvBishop, 2343, -20000, 20000, 200);
SEARCH_PARAM(mvvRook, 4711, -20000, 20000, 200);
SEARCH_PARAM(mvvQueen, 7060, -20000, 20000, 200);

SEARCH_PARAM(fpMaxDepth, 8, 4, 9, 1);
SEARCH_PARAM(fpBaseMargin, 140, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 135, 10, 180, 12);
SEARCH_PARAM(fpHistDivisor, 400, 256, 512, 16);

SEARCH_PARAM(noisyFpMaxDepth, 5, 2, 9, 1);
SEARCH_PARAM(noisyFPBaseMargin, 2, -100, 200, 10);
SEARCH_PARAM(noisyFpDepthMargin, 115, 10, 360, 12);
SEARCH_PARAM(noisyFpHistDivisor, 249, 100, 512, 16);

SEARCH_PARAM(lmpImpBase, 601, 128, 2048, 64);
SEARCH_PARAM(lmpImpDepth, 343, 64, 512, 48);
SEARCH_PARAM(lmpNonImpBase, 529, 128, 2048, 64);
SEARCH_PARAM(lmpNonImpDepth, 66, 64, 512, 48);

SEARCH_PARAM(seePruneMarginNoisy, -100, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -67, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 105, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 29, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 7, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1688, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 5, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 52, 24, 128, 4);
SEARCH_PARAM(sBetaScaleFormerPV, 21, 10, 128, 4);
SEARCH_PARAM(doubleExtMargin, 10, 0, 40, 2);
SEARCH_PARAM(tripleExtMargin, 124, 0, 200, 8);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 774, -512, 2048, 110, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrScale, 425, 300, 600, 50, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 8846, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6837, 2048, 16384, 512);

SEARCH_PARAM(lmrNonImp, 1497, 0, 3072, 256);
SEARCH_PARAM(lmrNoisyTTMove, 1113, 0, 3072, 256);
SEARCH_PARAM(lmrTTPV, 988, 0, 3072, 256);
SEARCH_PARAM(lmrTTPVNonFailLow, 524, 0, 3072, 256);
SEARCH_PARAM(lmrGivesCheck, 349, 0, 3072, 256);
SEARCH_PARAM(lmrInCheck, 575, 0, 3072, 256);
SEARCH_PARAM(lmrCorrplexity, 605, 0, 3072, 256);
SEARCH_PARAM(lmrCutnode, 1720, 0, 3072, 256);
SEARCH_PARAM(lmrFailHighCount, 1068, 0, 3072, 256);

SEARCH_PARAM(doDeeperMarginBase, 36, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 141, 32, 384, 16);

SEARCH_PARAM(doShallowerMargin, 8, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 78, 0, 250, 16);

}
