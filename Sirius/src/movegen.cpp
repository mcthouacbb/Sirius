#include "movegen.h"
#include "attacks.h"


bool canTakeEP(BitBoard eastRay, BitBoard westRay, BitBoard kingBB, BitBoard enemyAttackers, BitBoard allPieces)
{
    BitBoard eastBlockers = eastRay & allPieces;
    if (eastBlockers == 0)
        return true;

    BitBoard westBlockers = westRay & allPieces;
    if (westBlockers == 0)
        return true;

    BitBoard closestEastBlocker = extractLSB(eastBlockers);
    BitBoard other;
    if (closestEastBlocker & kingBB)
    {
        other = enemyAttackers;
    }
    else if (closestEastBlocker & enemyAttackers)
    {
        other = kingBB;
    }
    else
    {
        return true;
    }

    uint32_t closestWestBlocker = getMSB(westBlockers);
    if ((1ull << closestWestBlocker) & other)
        return false;
    return true;
}

template<MoveGenType type, Color color>
void genMoves(const Board& board, MoveList& moves);

template<MoveGenType type>
void genMoves(const Board& board, MoveList& moves)
{
    assert(moves.size() == 0);
    if (board.sideToMove() == Color::WHITE)
    {
        genMoves<type, Color::WHITE>(board, moves);
    }
    else
    {
        genMoves<type, Color::BLACK>(board, moves);
    }
}

template void genMoves<MoveGenType::LEGAL>(const Board& board, MoveList& moves);
template void genMoves<MoveGenType::CAPTURES>(const Board& board, MoveList& moves);

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, MoveList& moves, BitBoard checkBB);

template<MoveGenType type, Color color>
void genQueenMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genRookMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genBishopMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genKnightMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask);

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask);



template<MoveGenType type, Color color>
void genMoves(const Board& board, MoveList& moves)
{
    BitBoard checkers = board.checkers();
    if ((checkers & (checkers - 1)) == 0)
    {
        uint32_t kingIdx = getLSB(board.getPieces(color, PieceType::KING));
        BitBoard moveMask = checkers ? attacks::moveMaskBB(kingIdx, getLSB(checkers)) : ~0ull;
        // printBB(moveMask);
        if constexpr (type == MoveGenType::CAPTURES)
            moveMask &= board.getColor(flip(color));
        BitBoard pinned = board.checkBlockers(color);
        genPawnMoves<type, color>(board, moves, pinned, moveMask);
        genKnightMoves<type, color>(board, moves, pinned, moveMask);
        genBishopMoves<type, color>(board, moves, pinned, moveMask);
        genRookMoves<type, color>(board, moves, pinned, moveMask);
        genQueenMoves<type, color>(board, moves, pinned, moveMask);
    }
    genKingMoves<type, color>(board, moves);
}

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, MoveList& moves)
{
    BitBoard usBB = board.getColor(color);
    BitBoard oppBB = board.getColor(flip(color));
    BitBoard kingBB = board.getPieces(color, PieceType::KING);
    BitBoard checkBlockers = board.getAllPieces() ^ kingBB;
    uint32_t kingIdx = getLSB(kingBB);

    BitBoard kingAttacks = attacks::getKingAttacks(kingIdx);
    kingAttacks &= ~usBB;
    if constexpr (type == MoveGenType::CAPTURES)
        kingAttacks &= oppBB;
    while (kingAttacks)
    {
        uint32_t dst = popLSB(kingAttacks);
        if (!board.squareAttacked(flip(color), dst, checkBlockers))
            moves.push_back(Move(kingIdx, dst, MoveType::NONE));
    }

    BitBoard occupied = board.getAllPieces();

    uint32_t kscBit = 1 << (2 * static_cast<int>(color));
    uint32_t qscBit = 2 << (2 * static_cast<int>(color));
    if constexpr (type == MoveGenType::LEGAL)
    {
        bool ksChecked = false;
        BitBoard checkSquares = attacks::kscCheckSquares<color>();
        while (checkSquares)
        {
            uint32_t square = popLSB(checkSquares);
            if (board.squareAttacked(flip(color), square))
            {
                ksChecked = true;
                break;
            }
        }

        if (!ksChecked && !(attacks::kscBlockSquares<color>() & occupied) && (board.castlingRights() & kscBit))
        {
            moves.push_back(Move(kingIdx, kingIdx + 2, MoveType::CASTLE));
        }

        bool qsChecked = false;
        checkSquares = attacks::qscCheckSquares<color>();
        while (checkSquares)
        {
            uint32_t square = popLSB(checkSquares);
            if (board.squareAttacked(flip(color), square))
            {
                qsChecked = true;
                break;
            }
        }

        if (!(qsChecked) && !(attacks::qscBlockSquares<color>() & occupied) && (board.castlingRights() & qscBit))
        {
            moves.push_back(Move(kingIdx, kingIdx - 2, MoveType::CASTLE));
        }
    }
}

template<MoveGenType type, Color color>
void genQueenMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask)
{
    BitBoard usBB = board.getColor(color);
    BitBoard queenBB = board.getPieces(color, PieceType::QUEEN);
    BitBoard allPieces = board.getAllPieces();

    uint32_t kingIdx = getLSB(board.getPieces(color, PieceType::KING));

    while (queenBB)
    {
        uint32_t queenIdx = popLSB(queenBB);
        BitBoard queenAttacks = attacks::getQueenAttacks(queenIdx, allPieces);
        queenAttacks &= ~usBB;
        queenAttacks &= moveMask;
        if (pinned & (1ull << queenIdx))
        {
            queenAttacks &= attacks::pinRayBB(kingIdx, queenIdx);
        }
        while (queenAttacks)
        {
            uint32_t dst = popLSB(queenAttacks);
            moves.push_back(Move(queenIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genRookMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask)
{
    BitBoard usBB = board.getColor(color);
    BitBoard rookBB = board.getPieces(color, PieceType::ROOK);
    BitBoard allPieces = board.getAllPieces();

    uint32_t kingIdx = getLSB(board.getPieces(color, PieceType::KING));

    while (rookBB)
    {
        uint32_t rookIdx = popLSB(rookBB);
        BitBoard rookAttacks = attacks::getRookAttacks(rookIdx, allPieces);
        rookAttacks &= ~usBB;
        rookAttacks &= moveMask;
        if (pinned & (1ull << rookIdx))
        {
            rookAttacks &= attacks::pinRayBB(kingIdx, rookIdx);
        }
        while (rookAttacks)
        {
            uint32_t dst = popLSB(rookAttacks);
            moves.push_back(Move(rookIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genBishopMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask)
{
    BitBoard usBB = board.getColor(color);
    BitBoard bishopBB = board.getPieces(color, PieceType::BISHOP);
    BitBoard allPieces = board.getAllPieces();

    uint32_t kingIdx = getLSB(board.getPieces(color, PieceType::KING));

    while (bishopBB)
    {
        uint32_t bishopIdx = popLSB(bishopBB);
        BitBoard bishopAttacks = attacks::getBishopAttacks(bishopIdx, allPieces);
        bishopAttacks &= ~usBB;
        bishopAttacks &= moveMask;
        if (pinned & (1ull << bishopIdx))
        {
            bishopAttacks &= attacks::pinRayBB(kingIdx, bishopIdx);
        }
        while (bishopAttacks)
        {
            uint32_t dst = popLSB(bishopAttacks);
            moves.push_back(Move(bishopIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genKnightMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask)
{
    BitBoard usBB = board.getColor(color);
    BitBoard knightBB = board.getPieces(color, PieceType::KNIGHT);
    knightBB &= ~pinned;

    while (knightBB)
    {
        uint32_t knightIdx = popLSB(knightBB);
        BitBoard knightAttacks = attacks::getKnightAttacks(knightIdx);
        knightAttacks &= moveMask;
        knightAttacks &= ~usBB;
        while (knightAttacks)
        {
            uint32_t dst = popLSB(knightAttacks);
            moves.push_back(Move(knightIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, BitBoard pinned, BitBoard moveMask)
{
    // BitBoard usBB = board.getColor(color);
    BitBoard oppBB = board.getColor(flip(color));
    BitBoard pawns = board.getPieces(color, PieceType::PAWN);
    BitBoard allPieces = board.getAllPieces();

    uint32_t kingIdx = getLSB(board.getPieces(color, PieceType::KING));

    BitBoard pinnedPawns = pinned & pawns;

    pawns ^= pinnedPawns;


    if constexpr (type == MoveGenType::LEGAL)
    {
        BitBoard pawnPushes = attacks::getPawnBBPushes<color>(pawns | (pinnedPawns & (FILE_A << (kingIdx & 7))));
        pawnPushes &= ~allPieces;

        BitBoard doublePushes = attacks::getPawnBBPushes<color>(pawnPushes & nthRank<color, 2>());
        doublePushes &= ~allPieces;
        doublePushes &= moveMask;

        pawnPushes &= moveMask;

        BitBoard promotions = pawnPushes & nthRank<color, 7>();

        pawnPushes ^= promotions;

        while (pawnPushes)
        {
            uint32_t push = popLSB(pawnPushes);
            moves.push_back(Move(push - attacks::pawnPushOffset<color>(), push, MoveType::NONE));
        }

        while (doublePushes)
        {
            uint32_t dPush = popLSB(doublePushes);
            moves.push_back(Move(dPush - 2 * attacks::pawnPushOffset<color>(), dPush, MoveType::NONE));
        }

        while (promotions)
        {
            uint32_t promotion = popLSB(promotions);
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::KNIGHT));
        }
    }
    else if constexpr (type == MoveGenType::CAPTURES)
    {
        BitBoard pawnPushes = attacks::getPawnBBPushes<color>(pawns | (pinnedPawns & (FILE_A << (kingIdx & 7))));
        pawnPushes &= ~allPieces;
        pawnPushes &= moveMask;

        BitBoard promotions = pawnPushes & nthRank<color, 7>();

        while (promotions)
        {
            uint32_t promotion = popLSB(promotions);
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::KNIGHT));
        }
    }



    BitBoard eastCaptures = attacks::getPawnBBEastAttacks<color>(pawns | (pinnedPawns & attacks::getRay(kingIdx, attacks::pawnEastCaptureDir<color>())));
    if (board.epSquare() != -1)
    {
        if ((eastCaptures & (1ull << board.epSquare())) && ((1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
        {
            if (canTakeEP(
                attacks::getRay(board.epSquare() - attacks::pawnPushOffset<color>(), Direction::EAST),
                attacks::getRay(board.epSquare() - 1 - attacks::pawnPushOffset<color>(), Direction::WEST),
                1ull << kingIdx,
                (board.getPieces(PieceType::ROOK) & oppBB) | (board.getPieces(PieceType::QUEEN) & oppBB),
                allPieces
            ))
            {
                moves.push_back(Move(board.epSquare() - attacks::pawnPushOffset<color>() - 1, board.epSquare(), MoveType::ENPASSANT));
            }
        }
    }
    eastCaptures &= moveMask;

    eastCaptures &= oppBB;

    BitBoard promotions = eastCaptures & nthRank<color, 7>();
    eastCaptures ^= promotions;

    while (eastCaptures)
    {
        uint32_t capture = popLSB(eastCaptures);
        moves.push_back(Move(capture - attacks::pawnPushOffset<color>() - 1, capture, MoveType::NONE));
    }

    while (promotions)
    {
        uint32_t promotion = popLSB(promotions);
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }


    BitBoard westCaptures = attacks::getPawnBBWestAttacks<color>(pawns | (pinnedPawns & attacks::getRay(kingIdx, attacks::pawnWestCaptureDir<color>())));
    if (board.epSquare() != -1)
    {
        if ((westCaptures & (1ull << board.epSquare())) && ((1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
        {
            if (canTakeEP(
                attacks::getRay(board.epSquare() + 1 - attacks::pawnPushOffset<color>(), Direction::EAST),
                attacks::getRay(board.epSquare() - attacks::pawnPushOffset<color>(), Direction::WEST),
                1ull << kingIdx,
                (board.getPieces(PieceType::ROOK) & oppBB) | (board.getPieces(PieceType::QUEEN) & oppBB),
                allPieces
            ))
            {
                moves.push_back(Move(board.epSquare() - attacks::pawnPushOffset<color>() + 1, board.epSquare(), MoveType::ENPASSANT));
            }
        }
    }
    westCaptures &= moveMask;

    westCaptures &= oppBB;

    promotions = westCaptures & nthRank<color, 7>();
    westCaptures ^= promotions;

    while (westCaptures)
    {
        uint32_t capture = popLSB(westCaptures);
        moves.push_back(Move(capture - attacks::pawnPushOffset<color>() + 1, capture, MoveType::NONE));
    }

    while (promotions)
    {
        uint32_t promotion = popLSB(promotions);
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }
}
