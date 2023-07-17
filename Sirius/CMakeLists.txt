set(SRCS
	"src/attacks.cpp"
	"src/attacks.h"
	"src/bitboard.h"
	"src/board.cpp"
	"src/board.h"
	"src/defs.h"
	"src/main.cpp"
	"src/move_ordering.cpp"
	"src/move_ordering.h"
	"src/movegen.cpp"
	"src/movegen.h"
	"src/search.cpp"
	"src/search.h"

	"src/eval/draw.h"
	"src/eval/eval.cpp"
	"src/eval/eval.h"
	"src/eval/eval_state.h"
	"src/eval/material.cpp"
	"src/eval/material.h"
	"src/eval/phase.cpp"
	"src/eval/phase.h"
	"src/eval/psqt.h"

	"src/comm/command.cpp"
	"src/comm/command.h"
	"src/comm/fen.cpp"
	"src/comm/fen.h"
	"src/comm/move.cpp"
	"src/comm/move.h"
)

add_executable(sirius ${SRCS})

target_compile_features(sirius PRIVATE cxx_std_20)

# for Visual Studio/MSVC
set_target_properties(sirius PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})