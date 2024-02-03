#include "movegen.h"
#include "attacks.h"

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

template<>
void genMoves<MoveGenType::LEGAL>(const Board& board, MoveList& moves)
{
    genMoves<MoveGenType::NOISY_QUIET>(board, moves);
    size_t j = 0;
    for (size_t i = 0; i < moves.size(); i++)
    {
        if (board.isLegal(moves[i]))
            moves[j++] = moves[i];
    }
    moves.resize(j);
}

template void genMoves<MoveGenType::NOISY>(const Board& board, MoveList& moves);
template void genMoves<MoveGenType::NOISY_QUIET>(const Board& board, MoveList& moves);

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, MoveList& moves, BitBoard checkBB);

template<MoveGenType type, Color color>
void genQueenMoves(const Board& board, MoveList& moves, BitBoard moveMask);

template<MoveGenType type, Color color>
void genRookMoves(const Board& board, MoveList& moves, BitBoard moveMask);

template<MoveGenType type, Color color>
void genBishopMoves(const Board& board, MoveList& moves, BitBoard moveMask);

template<MoveGenType type, Color color>
void genKnightMoves(const Board& board, MoveList& moves, BitBoard moveMask);

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, BitBoard moveMask);



template<MoveGenType type, Color color>
void genMoves(const Board& board, MoveList& moves)
{
    BitBoard checkers = board.checkers();
    if ((checkers & (checkers - 1)) == 0)
    {
        uint32_t kingIdx = getLSB(board.getPieces(color, PieceType::KING));
        BitBoard moveMask = ~board.getColor(color) & (checkers ? attacks::moveMask(kingIdx, getLSB(checkers)) : ~0ull);
        genPawnMoves<type, color>(board, moves, moveMask);
        if constexpr (type == MoveGenType::NOISY)
            moveMask &= board.getColor(flip<color>());
        genKnightMoves<type, color>(board, moves, moveMask);
        genBishopMoves<type, color>(board, moves, moveMask);
        genRookMoves<type, color>(board, moves, moveMask);
        genQueenMoves<type, color>(board, moves, moveMask);
    }
    genKingMoves<type, color>(board, moves);
}

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, MoveList& moves)
{
    BitBoard usBB = board.getColor(color);
    BitBoard oppBB = board.getColor(flip<color>());
    BitBoard kingBB = board.getPieces(color, PieceType::KING);
    uint32_t kingIdx = getLSB(kingBB);

    BitBoard kingAttacks = attacks::kingAttacks(kingIdx);
    kingAttacks &= ~usBB;
    if constexpr (type == MoveGenType::NOISY)
        kingAttacks &= oppBB;
    while (kingAttacks)
    {
        uint32_t dst = popLSB(kingAttacks);
        moves.push_back(Move(kingIdx, dst, MoveType::NONE));
    }

    if constexpr (type == MoveGenType::NOISY_QUIET)
    {
        if (!board.checkers())
        {
            uint32_t kscBit = 1 << (2 * static_cast<int>(color));
            uint32_t qscBit = 2 << (2 * static_cast<int>(color));

            if (!(attacks::kscBlockSquares<color>() & board.getAllPieces()) && (board.castlingRights() & kscBit))
            {
                moves.push_back(Move(kingIdx, kingIdx + 2, MoveType::CASTLE));
            }

            if (!(attacks::qscBlockSquares<color>() & board.getAllPieces()) && (board.castlingRights() & qscBit))
            {
                moves.push_back(Move(kingIdx, kingIdx - 2, MoveType::CASTLE));
            }
        }
    }
}

template<MoveGenType type, Color color>
void genQueenMoves(const Board& board, MoveList& moves, BitBoard moveMask)
{
    BitBoard queenBB = board.getPieces(color, PieceType::QUEEN);
    BitBoard allPieces = board.getAllPieces();

    while (queenBB)
    {
        uint32_t queenIdx = popLSB(queenBB);
        BitBoard queenAttacks = attacks::queenAttacks(queenIdx, allPieces) & moveMask;
        while (queenAttacks)
        {
            uint32_t dst = popLSB(queenAttacks);
            moves.push_back(Move(queenIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genRookMoves(const Board& board, MoveList& moves, BitBoard moveMask)
{
    BitBoard rookBB = board.getPieces(color, PieceType::ROOK);
    BitBoard allPieces = board.getAllPieces();

    while (rookBB)
    {
        uint32_t rookIdx = popLSB(rookBB);
        BitBoard rookAttacks = attacks::rookAttacks(rookIdx, allPieces) & moveMask;
        while (rookAttacks)
        {
            uint32_t dst = popLSB(rookAttacks);
            moves.push_back(Move(rookIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genBishopMoves(const Board& board, MoveList& moves, BitBoard moveMask)
{
    BitBoard bishopBB = board.getPieces(color, PieceType::BISHOP);
    BitBoard allPieces = board.getAllPieces();

    while (bishopBB)
    {
        uint32_t bishopIdx = popLSB(bishopBB);
        BitBoard bishopAttacks = attacks::bishopAttacks(bishopIdx, allPieces) & moveMask;
        while (bishopAttacks)
        {
            uint32_t dst = popLSB(bishopAttacks);
            moves.push_back(Move(bishopIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genKnightMoves(const Board& board, MoveList& moves, BitBoard moveMask)
{
    BitBoard knightBB = board.getPieces(color, PieceType::KNIGHT);

    while (knightBB)
    {
        uint32_t knightIdx = popLSB(knightBB);
        BitBoard knightAttacks = attacks::knightAttacks(knightIdx) & moveMask;
        while (knightAttacks)
        {
            uint32_t dst = popLSB(knightAttacks);
            moves.push_back(Move(knightIdx, dst, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, BitBoard moveMask)
{
    BitBoard pawns = board.getPieces(color, PieceType::PAWN);
    BitBoard allPieces = board.getAllPieces();
    BitBoard oppBB = board.getColor(flip<color>());

    if constexpr (type == MoveGenType::LEGAL || type == MoveGenType::NOISY_QUIET)
    {
        BitBoard pawnPushes = attacks::pawnPushes<color>(pawns);
        pawnPushes &= ~allPieces;

        BitBoard doublePushes = attacks::pawnPushes<color>(pawnPushes & nthRank<color, 2>());
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
    else if constexpr (type == MoveGenType::NOISY)
    {
        BitBoard pawnPushes = attacks::pawnPushes<color>(pawns);
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



    BitBoard eastCaptures = attacks::pawnEastAttacks<color>(pawns);
    if (board.epSquare() != -1 && (eastCaptures & (1ull << board.epSquare())) && ((1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
        moves.push_back(Move(board.epSquare() - attacks::pawnPushOffset<color>() - 1, board.epSquare(), MoveType::ENPASSANT));

    eastCaptures &= oppBB;
    eastCaptures &= moveMask;

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


    BitBoard westCaptures = attacks::pawnWestAttacks<color>(pawns);
    if (board.epSquare() != -1 && (westCaptures & (1ull << board.epSquare())) && ((1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
        moves.push_back(Move(board.epSquare() - attacks::pawnPushOffset<color>() + 1, board.epSquare(), MoveType::ENPASSANT));

    westCaptures &= oppBB;
    westCaptures &= moveMask;

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
