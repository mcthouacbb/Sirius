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

SEARCH_PARAM(hardTimeScale, 59, 20, 100, 5);
SEARCH_PARAM(softTimeScale, 68, 30, 100, 5);
SEARCH_PARAM(incrementScale, 93, 25, 100, 5);
SEARCH_PARAM(baseTimeScale, 20, 8, 40, 1);

SEARCH_PARAM(nodeTMBase, 138, 110, 200, 5);
SEARCH_PARAM(nodeTMScale, 154, 100, 200, 5);

SEARCH_PARAM(bmStabilityBase, 92, 20, 200, 10);
SEARCH_PARAM(bmStabilityMin, 93, 60, 120, 15);
SEARCH_PARAM(bmStabilityScale, 954, 200, 1600, 40);
SEARCH_PARAM(bmStabilityOffset, 245, 100, 1000, 40);
SEARCH_PARAM(bmStabilityPower, -158, -250, -120, 8);

SEARCH_PARAM(maxHistBonus, 1940, 512, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 415, 1, 1536, 64);
SEARCH_PARAM(histBonusLinear, 218, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 147, -384, 768, 64);

SEARCH_PARAM(maxHistMalus, 1059, 512, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 299, 1, 1536, 64);
SEARCH_PARAM(histMalusLinear, 315, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, -4, -384, 768, 64);

SEARCH_PARAM(histBetaMargin, 42, 10, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 8441, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 2038, 1024, 8192, 64);

SEARCH_PARAM(pawnCorrWeight, 543, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 470, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 284, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 266, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 315, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 397, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 458, 96, 768, 64);
SEARCH_PARAM(contCorr3Weight, 215, 96, 768, 64);
SEARCH_PARAM(contCorr4Weight, 387, 96, 768, 64);
SEARCH_PARAM(contCorr5Weight, 295, 96, 768, 64);
SEARCH_PARAM(contCorr6Weight, 204, 96, 768, 64);
SEARCH_PARAM(contCorr7Weight, 203, 96, 768, 64);

SEARCH_PARAM(aspInitDelta, 9, 5, 30, 2);
SEARCH_PARAM(minAspDepth, 6, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 61, 1, 512, 24);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 31, 20, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 76, 50, 100, 8);
SEARCH_PARAM(rfpOppEasyCapture, 22, 5, 100, 8);
SEARCH_PARAM(rfpOppWorsening, 9, 5, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 409, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 450, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalMargin, 35, 0, 100, 10);
SEARCH_PARAM(nmpStaticEvalBaseMargin, 190, 50, 300, 10);
SEARCH_PARAM(nmpStaticEvalDepthMargin, 19, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 1327, 512, 2048, 64);
SEARCH_PARAM(nmpDepthReductionScale, 69, 32, 96, 12);
SEARCH_PARAM(nmpEvalReductionScale, 211, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 163, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(mvvPawn, 936, -20000, 20000, 200);
SEARCH_PARAM(mvvKnight, 2451, -20000, 20000, 200);
SEARCH_PARAM(mvvBishop, 2403, -20000, 20000, 200);
SEARCH_PARAM(mvvRook, 4745, -20000, 20000, 200);
SEARCH_PARAM(mvvQueen, 7195, -20000, 20000, 200);

SEARCH_PARAM(fpMaxDepth, 8, 4, 9, 1);
SEARCH_PARAM(fpBaseMargin, 147, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 135, 10, 180, 12);
SEARCH_PARAM(fpHistDivisor, 381, 256, 512, 16);

SEARCH_PARAM(noisyFpMaxDepth, 5, 2, 9, 1);
SEARCH_PARAM(noisyFPBaseMargin, 1, -100, 200, 10);
SEARCH_PARAM(noisyFpDepthMargin, 116, 10, 360, 12);
SEARCH_PARAM(noisyFpHistDivisor, 235, 100, 512, 16);

SEARCH_PARAM(lmpImpBase, 528, 128, 2048, 64);
SEARCH_PARAM(lmpImpDepth, 341, 64, 512, 48);
SEARCH_PARAM(lmpNonImpBase, 583, 128, 2048, 64);
SEARCH_PARAM(lmpNonImpDepth, 177, 64, 512, 48);

SEARCH_PARAM(seePruneMarginNoisy, -94, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -60, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 101, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 31, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 7, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1563, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 5, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 56, 24, 128, 4);
SEARCH_PARAM(sBetaScaleFormerPV, 17, 10, 128, 4);
SEARCH_PARAM(doubleExtMargin, 9, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);
SEARCH_PARAM(lmrCorrplexityMargin, 89, 40, 120, 5);

SEARCH_PARAM_CALLBACK(lmrBase, 744, -512, 2048, 110, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrScale, 382, 300, 600, 50, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 9344, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6364, 2048, 16384, 512);

SEARCH_PARAM(lmrNonImp, 1644, 0, 2048, 256);
SEARCH_PARAM(lmrNoisyTTMove, 1200, 0, 2048, 256);
SEARCH_PARAM(lmrTTPV, 1089, 0, 2048, 256);
SEARCH_PARAM(lmrTTPVNonFailLow, 483, 0, 2048, 256);
SEARCH_PARAM(lmrGivesCheck, 528, 0, 2048, 256);
SEARCH_PARAM(lmrInCheck, 584, 0, 2048, 256);
SEARCH_PARAM(lmrCorrplexity, 497, 0, 2048, 256);
SEARCH_PARAM(lmrCutnode, 1841, 0, 2048, 256);
SEARCH_PARAM(lmrFailHighCount, 974, 0, 2048, 256);

SEARCH_PARAM(doDeeperMarginBase, 34, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 141, 32, 384, 16);

SEARCH_PARAM(doShallowerMargin, 7, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 77, 0, 250, 16);

}
