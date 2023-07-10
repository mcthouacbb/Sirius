set(SRCS
	"src/sirius.h"

	"src/attacks.cpp"
	"src/attacks.h"
	"src/bitboard.h"
	"src/board.cpp"
	"src/board.h"
	"src/book.cpp"
	"src/book.h"
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
	"src/time_man.cpp"
	"src/time_man.h"
	"src/tt.cpp"
	"src/tt.h"
	"src/zobrist.cpp"
	"src/zobrist.h"

	"src/eval/draw.cpp"
	"src/eval/draw.h"
	"src/eval/eval.cpp"
	"src/eval/eval.h"
	"src/eval/eval_state.h"
	"src/eval/material.cpp"
	"src/eval/material.h"
	"src/eval/phase.cpp"
	"src/eval/phase.h"
	"src/eval/psqt.cpp"
	"src/eval/psqt.h"

	"src/comm/cmdline.cpp"
	"src/comm/cmdline.h"
	"src/comm/debug.cpp"
	"src/comm/debug.h"
	"src/comm/fen.cpp"
	"src/comm/fen.h"
	"src/comm/icomm.cpp"
	"src/comm/icomm.h"
	"src/comm/input_queue.cpp"
	"src/comm/input_queue.h"
	"src/comm/move.cpp"
	"src/comm/move.h"
	"src/comm/pgn.cpp"
	"src/comm/pgn.h"
	"src/comm/uci.cpp"
	"src/comm/uci.h"

	"src/tune/error.cpp"
	"src/tune/error.h"
	"src/tune/eval_params.cpp"
	"src/tune/eval_params.h"
	"src/tune/optimize.cpp"
	"src/tune/optimize.h"
	"src/tune/pos.cpp"
	"src/tune/pos.h"
)

add_executable(sirius ${SRCS})

target_compile_features(sirius PRIVATE cxx_std_20)

# for Visual Studio/MSVC
set_target_properties(sirius PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})