namespace search
{

constexpr uint32_t TIME_CHECK_INTERVAL = 2048;

constexpr int ASP_INIT_DELTA = 25;

constexpr int MIN_IIR_DEPTH = 4;

constexpr int RFP_MAX_DEPTH = 8;
constexpr int RFP_IMPROVING_FACTOR = 30;
constexpr int RFP_MARGIN = 80;

constexpr int NMP_MIN_DEPTH = 2;
constexpr int NMP_BASE_REDUCTION = 3;
constexpr int NMP_DEPTH_REDUCTION = 3;

constexpr int FP_BASE_MARGIN = 120;
constexpr int FP_DEPTH_MARGIN = 75;
constexpr int FP_MAX_DEPTH = 6;

constexpr int LMP_MAX_DEPTH = 8;
constexpr int LMP_MIN_MOVES_BASE = 3;

constexpr int MAX_SEE_PRUNE_DEPTH = 8;
constexpr int SEE_PRUNE_MARGIN_NOISY = -100;
constexpr int SEE_PRUNE_MARGIN_QUIET = -60;

constexpr int MAX_HIST_PRUNING_DEPTH = 4;
constexpr int HIST_PRUNING_MARGIN = 1536;

constexpr int LMR_MIN_DEPTH = 3;
constexpr int LMR_MIN_MOVES_NON_PV = 3;
constexpr int LMR_MIN_MOVES_PV = 5;

constexpr double LMR_BASE = 0.77;
constexpr double LMR_DIVISOR = 2.36;
constexpr int LMR_HIST_DIVISOR = 8192;


}
