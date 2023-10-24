set(SRCS
    "src/sirius.h"

    "src/attacks.cpp"
    "src/attacks.h"
    "src/bench.cpp"
    "src/bench.h"
    "src/bitboard.h"
    "src/board.cpp"
    "src/board.h"
    "src/defs.h"
    "src/main.cpp"
    "src/misc.cpp"
    "src/misc.h"
    "src/move_ordering.cpp"
    "src/move_ordering.h"
    "src/movegen.cpp"
    "src/movegen.h"
    "src/prng.h"
    "src/search.cpp"
    "src/search.h"
    "src/search_params.h"
    "src/time_man.cpp"
    "src/time_man.h"
    "src/tt.cpp"
    "src/tt.h"
    "src/zobrist.cpp"
    "src/zobrist.h"

    "src/eval/combined_psqt.cpp"
    "src/eval/combined_psqt.h"
    "src/eval/draw.cpp"
    "src/eval/draw.h"
    "src/eval/eval.cpp"
    "src/eval/eval.h"
    "src/eval/eval_constants.h"
    "src/eval/eval_state.h"
    "src/eval/phase.cpp"
    "src/eval/phase.h"

    "src/comm/book.cpp"
    "src/comm/book.h"
    "src/comm/cmdline.cpp"
    "src/comm/cmdline.h"
    "src/comm/fen.cpp"
    "src/comm/fen.h"
    "src/comm/icomm.cpp"
    "src/comm/icomm.h"
    "src/comm/move.cpp"
    "src/comm/move.h"
    "src/comm/uci.cpp"
    "src/comm/uci.h"
)

add_executable(sirius ${SRCS})

target_compile_features(sirius PRIVATE cxx_std_20)

# for Visual Studio/MSVC
set_target_properties(sirius PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
