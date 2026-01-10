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

SEARCH_PARAM(hardTimeScale, 61, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 70, 30, 100, 5);
SEARCH_PARAM(incrementScale, 89, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 140, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 159, 100, 200, 5);

SEARCH_PARAM(bmStabilityBase, 77, 20, 200, 10);
SEARCH_PARAM(bmStabilityMin, 93, 60, 120, 15);
SEARCH_PARAM(bmStabilityScale, 969, 200, 1600, 40);
SEARCH_PARAM(bmStabilityOffset, 266, 100, 1000, 40);
SEARCH_PARAM(bmStabilityPower, -153, -250, -120, 8);

SEARCH_PARAM(maxHistBonus, 2036, 512, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 441, 1, 1536, 64);
SEARCH_PARAM(histBonusLinear, 219, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 98, -384, 768, 64);

SEARCH_PARAM(maxHistMalus, 1093, 512, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 292, 1, 1536, 64);
SEARCH_PARAM(histMalusLinear, 302, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 27, -384, 768, 64);

SEARCH_PARAM(histBetaMargin, 39, 10, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 8091, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 2009, 1024, 8192, 64);

SEARCH_PARAM(pawnCorrWeight, 418, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 387, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 273, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 228, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 266, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 404, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 349, 96, 768, 64);
SEARCH_PARAM(contCorr3Weight, 175, 96, 768, 64);
SEARCH_PARAM(contCorr4Weight, 230, 96, 768, 64);
SEARCH_PARAM(contCorr5Weight, 219, 96, 768, 64);
SEARCH_PARAM(contCorr6Weight, 163, 96, 768, 64);
SEARCH_PARAM(contCorr7Weight, 142, 96, 768, 64);
SEARCH_PARAM(highCorrplexityMargin, 87, 40, 120, 5);

SEARCH_PARAM(aspInitDelta, 10, 5, 30, 2);
SEARCH_PARAM(minAspDepth, 6, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 58, 1, 512, 24);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 27, 20, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 78, 50, 100, 8);
SEARCH_PARAM(rfpOppEasyCapture, 21, 5, 100, 8);
SEARCH_PARAM(rfpOppWorsening, 14, 5, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 410, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 456, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalMargin, 30, 0, 100, 10);
SEARCH_PARAM(nmpStaticEvalBaseMargin, 184, 50, 300, 10);
SEARCH_PARAM(nmpStaticEvalDepthMargin, 19, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 1320, 512, 2048, 64);
SEARCH_PARAM(nmpDepthReductionScale, 74, 32, 96, 12);
SEARCH_PARAM(nmpEvalReductionScale, 215, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 190, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(mvvPawn, 964, -20000, 20000, 200);
SEARCH_PARAM(mvvKnight, 2465, -20000, 20000, 200);
SEARCH_PARAM(mvvBishop, 2360, -20000, 20000, 200);
SEARCH_PARAM(mvvRook, 4725, -20000, 20000, 200);
SEARCH_PARAM(mvvQueen, 7181, -20000, 20000, 200);

SEARCH_PARAM(fpMaxDepth, 8, 4, 9, 1);
SEARCH_PARAM(fpBaseMargin, 146, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 128, 10, 180, 12);
SEARCH_PARAM(fpHistDivisor, 393, 256, 512, 16);

SEARCH_PARAM(noisyFpMaxDepth, 5, 2, 9, 1);
SEARCH_PARAM(noisyFPBaseMargin, 4, -100, 200, 10);
SEARCH_PARAM(noisyFpDepthMargin, 113, 10, 360, 12);
SEARCH_PARAM(noisyFpHistDivisor, 253, 100, 512, 16);

SEARCH_PARAM(lmpImpBase, 553, 128, 2048, 64);
SEARCH_PARAM(lmpImpDepth, 333, 64, 512, 48);
SEARCH_PARAM(lmpNonImpBase, 566, 128, 2048, 64);
SEARCH_PARAM(lmpNonImpDepth, 103, 64, 512, 48);

SEARCH_PARAM(seePruneMarginNoisy, -96, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -67, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 103, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 30, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 7, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1743, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 5, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 55, 24, 128, 4);
SEARCH_PARAM(sBetaScaleFormerPV, 20, 10, 128, 4);
SEARCH_PARAM(doubleExtMargin, 11, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);

SEARCH_PARAM_CALLBACK(lmrBase, 775, -512, 2048, 110, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrScale, 427, 300, 600, 50, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 9043, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6598, 2048, 16384, 512);

SEARCH_PARAM(lmrNonImp, 1478, 0, 2048, 256);
SEARCH_PARAM(lmrNoisyTTMove, 1082, 0, 2048, 256);
SEARCH_PARAM(lmrTTPV, 954, 0, 2048, 256);
SEARCH_PARAM(lmrTTPVNonFailLow, 484, 0, 2048, 256);
SEARCH_PARAM(lmrGivesCheck, 573, 0, 2048, 256);
SEARCH_PARAM(lmrInCheck, 592, 0, 2048, 256);
SEARCH_PARAM(lmrCorrplexity, 593, 0, 2048, 256);
SEARCH_PARAM(lmrCutnode, 1612, 0, 2048, 256);
SEARCH_PARAM(lmrFailHighCount, 1042, 0, 2048, 256);

SEARCH_PARAM(doDeeperMarginBase, 38, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 143, 32, 384, 16);

SEARCH_PARAM(doShallowerMargin, 8, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 80, 0, 250, 16);

}
