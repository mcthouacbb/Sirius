#include "endgame.h"

#include <algorithm>
#include <unordered_map>

namespace eval::endgames
{

int distToAnyCorner(Square sq)
{
	int rankDist = std::min(sq.rank(), 7 - sq.rank());
	int fileDist = std::min(sq.file(), 7 - sq.file());

	return rankDist + fileDist;
}

int trivialDraw(const Board&, Color)
{
	return 0;
}

int KBNvK(const Board& board, Color strongSide)
{
	Color weakSide = ~strongSide;
	Square bishop = board.pieces(strongSide, PieceType::BISHOP).lsb();
	Square ourKing = board.pieces(strongSide, PieceType::KING).lsb();
	Square theirKing = board.pieces(weakSide, PieceType::KING).lsb();

	int correctCornerDist = std::abs(7 - theirKing.rank() - theirKing.file());
	if (bishop.darkSquare())
		correctCornerDist = 7 - correctCornerDist;

	int cornerDist = distToAnyCorner(theirKing);

	int kingDist = Square::manhattan(ourKing, theirKing);

	return 10000 - kingDist * 20 - cornerDist * 20 - correctCornerDist * 200;
}

using Key = uint64_t;
using Map = std::unordered_map<Key, Endgame>;

Key genMaterialKey(std::string white, std::string black)
{
	assert(white.length() < 8);
	assert(black.length() < 8);

	std::transform(white.begin(), white.end(), white.begin(), [](auto c) { return std::toupper(c); });
	std::transform(black.begin(), black.end(), black.begin(), [](auto c) { return std::tolower(c); });

	std::string fen;
	fen += black;
	fen += static_cast<char>('0' + 8 - black.length());
	fen += "/8/8/8/8/8/8/";
	fen += white;
	fen += static_cast<char>('0' + 8 - white.length());
	fen += " w - - 0 1";

	Board board;
	board.setToFen(fen);

	return board.materialKey();
}

Map map;

void addEndgameFunc(Map& map, const std::string& strongCode, const std::string& weakCode, EndgameFunc* func)
{
	map.insert({genMaterialKey(strongCode, weakCode), Endgame(Color::WHITE, func)});
	map.insert({genMaterialKey(weakCode, strongCode), Endgame(Color::BLACK, func)});
}

Endgame* probe(const Board& board)
{
	auto it = map.find(board.materialKey());
	if (it != map.end())
		return &it->second;
	return nullptr;
}

void init()
{
	addEndgameFunc(map, "K", "K", trivialDraw);
	addEndgameFunc(map, "KN", "K", trivialDraw);
	addEndgameFunc(map, "KB", "K", trivialDraw);
	addEndgameFunc(map, "KN", "KN", trivialDraw);
	addEndgameFunc(map, "KB", "KN", trivialDraw);
	addEndgameFunc(map, "KNN", "K", trivialDraw);

	addEndgameFunc(map, "KBN", "K", KBNvK);
}


}
