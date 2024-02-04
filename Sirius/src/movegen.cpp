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
void genKingMoves(const Board& board, MoveList& moves);

template<MoveGenType type, Color color, PieceType piece>
void genPieceMoves(const Board& board, MoveList& moves, Bitboard moveMask);

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, Bitboard moveMask);



template<MoveGenType type, Color color>
void genMoves(const Board& board, MoveList& moves)
{
    Bitboard checkers = board.checkers();
    if (!checkers.multiple())
    {
        uint32_t kingIdx = board.getPieces(color, PieceType::KING).lsb();
        Bitboard moveMask = ~board.getColor(color) & (checkers ? attacks::moveMask(kingIdx, checkers.lsb()) : ~0ull);
        genPawnMoves<type, color>(board, moves, moveMask);
        if constexpr (type == MoveGenType::NOISY)
            moveMask &= board.getColor(flip<color>());
        genPieceMoves<type, color, PieceType::KNIGHT>(board, moves, moveMask);
        genPieceMoves<type, color, PieceType::BISHOP>(board, moves, moveMask);
        genPieceMoves<type, color, PieceType::ROOK>(board, moves, moveMask);
        genPieceMoves<type, color, PieceType::QUEEN>(board, moves, moveMask);
    }
    genKingMoves<type, color>(board, moves);
}

template<MoveGenType type, Color color>
void genKingMoves(const Board& board, MoveList& moves)
{
    Bitboard usBB = board.getColor(color);
    Bitboard oppBB = board.getColor(flip<color>());
    Bitboard kingBB = board.getPieces(color, PieceType::KING);
    uint32_t kingIdx = kingBB.lsb();

    Bitboard kingAttacks = attacks::kingAttacks(kingIdx);
    kingAttacks &= ~usBB;
    if constexpr (type == MoveGenType::NOISY)
        kingAttacks &= oppBB;
    while (kingAttacks)
    {
        uint32_t dst = kingAttacks.poplsb();
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

template<MoveGenType type, Color color, PieceType piece>
void genPieceMoves(const Board& board, MoveList& moves, Bitboard moveMask)
{
    Bitboard pieceBB = board.getPieces(color, piece);
    Bitboard allPieces = board.getAllPieces();

    while (pieceBB)
    {
        uint32_t from = pieceBB.poplsb();
        Bitboard attacks = attacks::pieceAttacks<piece>(from, allPieces) & moveMask;
        while (attacks)
        {
            uint32_t to = attacks.poplsb();
            moves.push_back(Move(from, to, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, Bitboard moveMask)
{
    Bitboard pawns = board.getPieces(color, PieceType::PAWN);
    Bitboard allPieces = board.getAllPieces();
    Bitboard oppBB = board.getColor(flip<color>());

    if constexpr (type == MoveGenType::NOISY_QUIET)
    {
        Bitboard pawnPushes = attacks::pawnPushes<color>(pawns);
        pawnPushes &= ~allPieces;

        Bitboard doublePushes = attacks::pawnPushes<color>(pawnPushes & Bitboard::nthRank<color, 2>());
        doublePushes &= ~allPieces;
        doublePushes &= moveMask;

        pawnPushes &= moveMask;

        Bitboard promotions = pawnPushes & Bitboard::nthRank<color, 7>();

        pawnPushes ^= promotions;

        while (pawnPushes)
        {
            uint32_t push = pawnPushes.poplsb();
            moves.push_back(Move(push - attacks::pawnPushOffset<color>(), push, MoveType::NONE));
        }

        while (doublePushes)
        {
            uint32_t dPush = doublePushes.poplsb();
            moves.push_back(Move(dPush - 2 * attacks::pawnPushOffset<color>(), dPush, MoveType::NONE));
        }

        while (promotions)
        {
            uint32_t promotion = promotions.poplsb();
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::KNIGHT));
        }
    }
    else if constexpr (type == MoveGenType::NOISY)
    {
        Bitboard pawnPushes = attacks::pawnPushes<color>(pawns);
        pawnPushes &= ~allPieces;
        pawnPushes &= moveMask;

        Bitboard promotions = pawnPushes & Bitboard::nthRank<color, 7>();

        while (promotions)
        {
            uint32_t promotion = promotions.poplsb();
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::KNIGHT));
        }
    }



    Bitboard eastCaptures = attacks::pawnEastAttacks<color>(pawns);
    if (board.epSquare() != -1 && (eastCaptures & Bitboard(1ull << board.epSquare())) && (Bitboard(1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
        moves.push_back(Move(board.epSquare() - attacks::pawnPushOffset<color>() - 1, board.epSquare(), MoveType::ENPASSANT));

    eastCaptures &= oppBB;
    eastCaptures &= moveMask;

    Bitboard promotions = eastCaptures & Bitboard::nthRank<color, 7>();
    eastCaptures ^= promotions;

    while (eastCaptures)
    {
        uint32_t capture = eastCaptures.poplsb();
        moves.push_back(Move(capture - attacks::pawnPushOffset<color>() - 1, capture, MoveType::NONE));
    }

    while (promotions)
    {
        uint32_t promotion = promotions.poplsb();
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }


    Bitboard westCaptures = attacks::pawnWestAttacks<color>(pawns);
    if (board.epSquare() != -1 && (westCaptures & Bitboard(1ull << board.epSquare())) && (Bitboard(1ull << (board.epSquare() - attacks::pawnPushOffset<color>())) & moveMask))
        moves.push_back(Move(board.epSquare() - attacks::pawnPushOffset<color>() + 1, board.epSquare(), MoveType::ENPASSANT));

    westCaptures &= oppBB;
    westCaptures &= moveMask;

    promotions = westCaptures & Bitboard::nthRank<color, 7>();
    westCaptures ^= promotions;

    while (westCaptures)
    {
        uint32_t capture = westCaptures.poplsb();
        moves.push_back(Move(capture - attacks::pawnPushOffset<color>() + 1, capture, MoveType::NONE));
    }

    while (promotions)
    {
        uint32_t promotion = promotions.poplsb();
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }
}
