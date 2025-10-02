# sp yoink

ifndef EXE
    EXE = sirius
endif

EXE_SUFFIX =
LDFLAGS = -fuse-ld=lld
ifeq ($(OS), Windows_NT)
	EXE_SUFFIX = .exe
endif

SOURCES := Sirius/src/attacks.cpp Sirius/src/bench.cpp Sirius/src/board.cpp Sirius/src/cuckoo.cpp \
	Sirius/src/history.cpp Sirius/src/main.cpp Sirius/src/misc.cpp Sirius/src/move_ordering.cpp \
	Sirius/src/movegen.cpp Sirius/src/search.cpp Sirius/src/search_params.cpp Sirius/src/time_man.cpp \
	Sirius/src/tt.cpp Sirius/src/datagen/datagen.cpp Sirius/src/datagen/extract.cpp Sirius/src/datagen/marlinformat.cpp \
	Sirius/src/datagen/stats.cpp Sirius/src/datagen/viriformat.cpp Sirius/src/eval/endgame.cpp Sirius/src/eval/eval.cpp \
	Sirius/src/eval/eval_state.cpp Sirius/src/eval/eval_terms.cpp Sirius/src/eval/pawn_structure.cpp \
	Sirius/src/eval/psqt_state.cpp Sirius/src/uci/fen.cpp Sirius/src/uci/move.cpp Sirius/src/uci/uci.cpp

HEADERS := Sirius/src/attacks.h Sirius/src/bench.h Sirius/src/bitboard.h Sirius/src/board.h \
	Sirius/src/castling.h Sirius/src/cuckoo.h Sirius/src/defs.h Sirius/src/history.h Sirius/src/misc.h \
	Sirius/src/move_ordering.h Sirius/src/movegen.h Sirius/src/search_params.h Sirius/src/search.h \
	Sirius/src/sirius.h Sirius/src/time_man.h Sirius/src/tt.h Sirius/src/zobrist.h Sirius/src/datagen/datagen.h \
	Sirius/src/datagen/extract.h Sirius/src/datagen/marlinformat.h Sirius/src/datagen/stats.h \
	Sirius/src/datagen/viriformat.h Sirius/src/util/enum_array.h Sirius/src/util/multi_array.h \
	Sirius/src/util/murmur.h Sirius/src/util/piece_set.h Sirius/src/util/prng.h Sirius/src/util/static_vector.h \
	Sirius/src/util/string_split.h Sirius/src/eval/combined_psqt.h Sirius/src/eval/endgame.h \
	Sirius/src/eval/eval_constants.h Sirius/src/eval/eval_state.h Sirius/src/eval/eval_terms.h \
	Sirius/src/eval/eval.h Sirius/src/eval/pawn_structure.h Sirius/src/eval/pawn_table.h \
	Sirius/src/eval/psqt_state.h Sirius/src/uci/fen.h Sirius/src/uci/move.h \
	Sirius/src/uci/uci_option.h Sirius/src/uci/uci.h Sirius/src/uci/wdl.h

CXX := clang++
CXXFLAGS := -std=c++20 -O3 -flto -DNDEBUG -march=native

$(EXE)$(EXE_SUFFIX): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXE)$(EXE_SUFFIX)

format:
	clang-format -i $(SOURCES) $(HEADERS)