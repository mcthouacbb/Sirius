namespace search
{

constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

constexpr int ASP_INIT_DELTA = 25;

constexpr int MIN_IIR_DEPTH = 4;

constexpr int RFP_MAX_DEPTH = 8;
constexpr int RFP_MARGIN = 75;

constexpr int NMP_MIN_DEPTH = 2;
constexpr int NMP_BASE_REDUCTION = 3;
constexpr int NMP_DEPTH_REDUCTION = 3;

constexpr int FP_BASE_MARGIN = 120;
constexpr int FP_DEPTH_MARGIN = 75;
constexpr int FP_MAX_DEPTH = 6;

constexpr int LMP_MAX_DEPTH = 8;
constexpr int LMP_MIN_MOVES_BASE = 3;

constexpr int MAX_SEE_PRUNE_DEPTH = 8;
constexpr int SEE_PRUNE_MARGIN = -100;

constexpr int LMR_MIN_DEPTH = 3;
constexpr int LMR_MIN_MOVES_NON_PV = 3;
constexpr int LMR_MIN_MOVES_PV = 5;

constexpr double LMR_BASE = 0.77;
constexpr double LMR_DIVISOR = 2.36;


}
