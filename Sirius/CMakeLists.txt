set(SRCS
    "src/sirius.h"

    "src/attacks.cpp"
    "src/attacks.h"
    "src/bench.cpp"
    "src/bench.h"
    "src/bitboard.h"
    "src/board.cpp"
    "src/board.h"
    "src/castling.h"
    "src/cuckoo.h"
    "src/cuckoo.cpp"
    "src/defs.h"
    "src/history.cpp"
    "src/history.h"
    "src/main.cpp"
    "src/misc.cpp"
    "src/misc.h"
    "src/move_ordering.cpp"
    "src/move_ordering.h"
    "src/movegen.cpp"
    "src/movegen.h"
    "src/search.cpp"
    "src/search.h"
    "src/search_params.cpp"
    "src/search_params.h"
    "src/time_man.cpp"
    "src/time_man.h"
    "src/tt.cpp"
    "src/tt.h"
    "src/zobrist.h"

    "src/eval/combined_psqt.h"
    "src/eval/endgame.h"
    "src/eval/endgame.cpp"
    "src/eval/eval.cpp"
    "src/eval/eval.h"
    "src/eval/eval_constants.h"
    "src/eval/eval_state.cpp"
    "src/eval/eval_state.h"
    "src/eval/eval_terms.cpp"
    "src/eval/eval_terms.h"
    "src/eval/pawn_structure.cpp"
    "src/eval/pawn_structure.h"
    "src/eval/pawn_table.h"
    "src/eval/psqt_state.cpp"
    "src/eval/psqt_state.h"

    "src/util/enum_array.h"
    "src/util/multi_array.h"
    "src/util/murmur.h"
    "src/util/static_vector.h"
    "src/util/piece_set.h"
    "src/util/prng.h"
    "src/util/string_split.h"

    "src/uci/fen.cpp"
    "src/uci/fen.h"
    "src/uci/move.cpp"
    "src/uci/move.h"
    "src/uci/uci.cpp"
    "src/uci/uci.h"
    "src/uci/uci_option.h"
    "src/uci/wdl.h"
)


if("${SIRIUS_EXE_EXTENSION}" STREQUAL "")
    message("none")
    set(SIRIUS_EXE_NAME "sirius")
else()
    set(SIRIUS_EXE_NAME "sirius-${SIRIUS_EXE_EXTENSION}")
endif()

add_executable(${SIRIUS_EXE_NAME} ${SRCS})

target_compile_features(${SIRIUS_EXE_NAME} PRIVATE cxx_std_20)

# for Visual Studio/MSVC
set_target_properties(${SIRIUS_EXE_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
