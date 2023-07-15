set(SRCS
	"src/attacks.cpp"
	"src/attacks.h"
	"src/bitboard.h"
	"src/board.cpp"
	"src/board.h"
	"src/defs.h"
	"src/main.cpp"
	"src/move.cpp"
	"src/move.h"
	"src/movegen.cpp"
	"src/movegen.h"
)

add_executable(sirius ${SRCS})

target_compile_features(sirius PRIVATE cxx_std_20)

# for Visual Studio/MSVC
set_target_properties(sirius PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})