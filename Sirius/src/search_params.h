#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <string>

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
SearchParam& addSearchParam(std::string name, int value, int min, int max, int step,
    std::function<void()> callback = std::function<void()>());
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
#define SEARCH_PARAM_CALLBACK(name, val, min, max, step, callback) \
    SEARCH_PARAM(name, val, min, max, step)
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

SEARCH_PARAM(maxHistBonus, 2048, 512, 3072, 256);
SEARCH_PARAM(histBonusQuadratic, 489, 1, 1536, 64);
SEARCH_PARAM(histBonusLinear, 210, 64, 384, 32);
SEARCH_PARAM(histBonusOffset, 168, -384, 768, 64);

SEARCH_PARAM(maxHistMalus, 1013, 512, 3072, 256);
SEARCH_PARAM(histMalusQuadratic, 315, 1, 1536, 64);
SEARCH_PARAM(histMalusLinear, 281, 64, 384, 32);
SEARCH_PARAM(histMalusOffset, 55, -384, 768, 64);

SEARCH_PARAM(histBetaMargin, 42, 10, 120, 5);

// 256 corrhist units = 1 eval unit
SEARCH_PARAM(maxCorrHist, 7904, 6144, 24576, 512);
SEARCH_PARAM(maxCorrHistUpdate, 1995, 1024, 8192, 64);

SEARCH_PARAM(pawnCorrWeight, 409, 96, 768, 64);
SEARCH_PARAM(nonPawnStmCorrWeight, 384, 96, 768, 64);
SEARCH_PARAM(nonPawnNstmCorrWeight, 239, 96, 768, 64);
SEARCH_PARAM(threatsCorrWeight, 287, 96, 768, 64);
SEARCH_PARAM(minorCorrWeight, 296, 96, 768, 64);
SEARCH_PARAM(majorCorrWeight, 380, 96, 768, 64);
SEARCH_PARAM(contCorr2Weight, 327, 96, 768, 64);
SEARCH_PARAM(contCorr3Weight, 225, 96, 768, 64);
SEARCH_PARAM(contCorr4Weight, 201, 96, 768, 64);
SEARCH_PARAM(contCorr5Weight, 214, 96, 768, 64);
SEARCH_PARAM(contCorr6Weight, 183, 96, 768, 64);
SEARCH_PARAM(contCorr7Weight, 132, 96, 768, 64);

SEARCH_PARAM(aspInitDelta, 9, 5, 30, 2);
SEARCH_PARAM(minAspDepth, 6, 3, 7, 1);
SEARCH_PARAM(aspWideningFactor, 56, 1, 512, 24);

SEARCH_PARAM(minIIRDepth, 4, 2, 9, 1);

SEARCH_PARAM(rfpMaxDepth, 8, 4, 10, 1);
SEARCH_PARAM(rfpImpMargin, 25, 20, 80, 8);
SEARCH_PARAM(rfpNonImpMargin, 78, 50, 100, 8);
SEARCH_PARAM(rfpOppWorsening, 9, 5, 100, 8);
SEARCH_PARAM(rfpHistDivisor, 408, 256, 512, 16);

SEARCH_PARAM(razoringMaxDepth, 3, 1, 5, 1);
SEARCH_PARAM(razoringMargin, 457, 250, 650, 10);

SEARCH_PARAM(nmpMinDepth, 2, 2, 5, 1);
SEARCH_PARAM(nmpEvalBaseMargin, 184, 50, 300, 10);
SEARCH_PARAM(nmpEvalDepthMargin, 22, 10, 50, 2);
SEARCH_PARAM(nmpBaseReduction, 1308, 512, 2048, 64);
SEARCH_PARAM(nmpDepthReductionScale, 73, 32, 96, 12);
SEARCH_PARAM(nmpEvalReductionScale, 213, 50, 300, 10);
SEARCH_PARAM(nmpMaxEvalReduction, 4, 2, 5, 1);

SEARCH_PARAM(probcutMinDepth, 5, 4, 7, 1);
SEARCH_PARAM(probcutBetaMargin, 200, 100, 300, 15);
SEARCH_PARAM(probcutReduction, 4, 3, 6, 1);

SEARCH_PARAM(mvvPawn, 961, -20000, 20000, 200);
SEARCH_PARAM(mvvKnight, 2551, -20000, 20000, 200);
SEARCH_PARAM(mvvBishop, 2382, -20000, 20000, 200);
SEARCH_PARAM(mvvRook, 4799, -20000, 20000, 200);
SEARCH_PARAM(mvvQueen, 7140, -20000, 20000, 200);

SEARCH_PARAM(fpMaxDepth, 8, 4, 9, 1);
SEARCH_PARAM(fpBaseMargin, 145, 60, 360, 12);
SEARCH_PARAM(fpDepthMargin, 130, 10, 180, 12);
SEARCH_PARAM(fpHistDivisor, 406, 256, 512, 16);

SEARCH_PARAM(noisyFpMaxDepth, 5, 2, 9, 1);
SEARCH_PARAM(noisyFPBaseMargin, 1, -100, 200, 10);
SEARCH_PARAM(noisyFpDepthMargin, 110, 10, 360, 12);
SEARCH_PARAM(noisyFpHistDivisor, 250, 100, 512, 16);

SEARCH_PARAM(lmpImpBase, 518, 128, 2048, 64);
SEARCH_PARAM(lmpImpDepth, 301, 64, 512, 48);
SEARCH_PARAM(lmpNonImpBase, 510, 128, 2048, 64);
SEARCH_PARAM(lmpNonImpDepth, 128, 64, 512, 48);

SEARCH_PARAM(seePruneMarginNoisy, -95, -120, -30, 6);
SEARCH_PARAM(seePruneMarginQuiet, -60, -120, -30, 6);
SEARCH_PARAM(seeCaptHistMax, 98, 50, 200, 6);
SEARCH_PARAM(seeCaptHistDivisor, 31, 16, 96, 2);

SEARCH_PARAM(maxHistPruningDepth, 7, 2, 8, 1);
SEARCH_PARAM(histPruningMargin, 1802, 512, 4096, 128);

SEARCH_PARAM(seMinDepth, 6, 4, 9, 1);
SEARCH_PARAM(seTTDepthMargin, 3, 2, 5, 1);
SEARCH_PARAM(sBetaScale, 52, 24, 128, 4);
SEARCH_PARAM(doubleExtMargin, 11, 0, 40, 2);

SEARCH_PARAM(lmrMinDepth, 3, 2, 5, 1);
SEARCH_PARAM(lmrMinMovesNonPv, 3, 1, 6, 1);
SEARCH_PARAM(lmrMinMovesPv, 4, 2, 8, 1);
SEARCH_PARAM(lmrFailHighCountMargin, 2, 2, 12, 1);
SEARCH_PARAM(lmrCorrplexityMargin, 92, 40, 120, 5);

SEARCH_PARAM_CALLBACK(lmrBase, 702, -512, 2048, 110, updateLmrTable);
SEARCH_PARAM_CALLBACK(lmrScale, 450, 300, 600, 50, updateLmrTable);
SEARCH_PARAM(lmrQuietHistDivisor, 8847, 4096, 16384, 512);
SEARCH_PARAM(lmrNoisyHistDivisor, 6342, 2048, 16384, 512);

SEARCH_PARAM(lmrNonImp, 1236, 0, 2048, 256);
SEARCH_PARAM(lmrNoisyTTMove, 1145, 0, 2048, 256);
SEARCH_PARAM(lmrTTPV, 1080, 0, 2048, 256);
SEARCH_PARAM(lmrGivesCheck, 781, 0, 2048, 256);
SEARCH_PARAM(lmrInCheck, 753, 0, 2048, 256);
SEARCH_PARAM(lmrCorrplexity, 783, 0, 2048, 256);
SEARCH_PARAM(lmrCutnode, 1254, 0, 2048, 256);
SEARCH_PARAM(lmrFailHighCount, 1007, 0, 2048, 256);

SEARCH_PARAM(doDeeperMarginBase, 37, 15, 55, 5);
SEARCH_PARAM(doDeeperMarginDepth, 139, 32, 384, 16);

SEARCH_PARAM(doShallowerMargin, 8, 2, 15, 1);

SEARCH_PARAM(qsFpMargin, 69, 0, 250, 16);

}
