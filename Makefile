# sp yoink

ifndef EXE
    EXE = sirius
endif

EXE_SUFFIX =
LDFLAGS = -fuse-ld=lld
ifeq ($(OS), Windows_NT)
	EXE_SUFFIX = .exe
endif

SOURCES := Sirius/src/attacks.cpp Sirius/src/bench.cpp Sirius/src/board.cpp Sirius/src/history.cpp Sirius/src/main.cpp Sirius/src/misc.cpp Sirius/src/move_ordering.cpp Sirius/src/movegen.cpp Sirius/src/search.cpp Sirius/src/search_params.cpp Sirius/src/time_man.cpp Sirius/src/tt.cpp Sirius/src/eval/draw.cpp Sirius/src/eval/eval.cpp Sirius/src/eval/eval_state.cpp Sirius/src/eval/eval_terms.cpp Sirius/src/eval/pawn_structure.cpp Sirius/src/eval/phase.cpp Sirius/src/eval/psqt_state.cpp Sirius/src/comm/book.cpp Sirius/src/comm/cmdline.cpp Sirius/src/comm/fen.cpp Sirius/src/comm/icomm.cpp Sirius/src/comm/move.cpp Sirius/src/comm/uci.cpp

CXX := clang++
CXXFLAGS := -std=c++20 -O3 -flto -DNDEBUG -march=native

$(EXE)$(EXE_SUFFIX): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXE)$(EXE_SUFFIX)