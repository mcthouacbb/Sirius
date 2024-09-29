#include "movegen.h"
#include "attacks.h"

template<MoveGenType type, Color color>
void genMoves(const Board& board, MoveList& moves);

template<MoveGenType type>
void genMoves(const Board& board, MoveList& moves)
{
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
template void genMoves<MoveGenType::QUIET>(const Board& board, MoveList& moves);

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
        Bitboard moveMask = ~board.pieces(color) &
            (checkers.any() ? attacks::moveMask(board.kingSq(color), checkers.lsb()) : Bitboard(~0ull));
        genPawnMoves<type, color>(board, moves, moveMask);
        if constexpr (type == MoveGenType::NOISY)
            moveMask &= board.pieces(~color);
        else if constexpr (type == MoveGenType::QUIET)
            moveMask &= ~board.pieces(~color);
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
    Bitboard usBB = board.pieces(color);
    Bitboard oppBB = board.pieces(~color);
    Square kingSq = board.kingSq(color);

    Bitboard kingAttacks = attacks::kingAttacks(kingSq);
    kingAttacks &= ~usBB;
    if constexpr (type == MoveGenType::NOISY)
        kingAttacks &= oppBB;
    else if constexpr (type == MoveGenType::QUIET)
        kingAttacks &= ~oppBB;
    while (kingAttacks.any())
    {
        Square dst = kingAttacks.poplsb();
        moves.push_back(Move(kingSq, dst, MoveType::NONE));
    }

    if constexpr (type == MoveGenType::NOISY_QUIET || type == MoveGenType::QUIET)
    {
        if (board.checkers().empty())
        {
            uint32_t kscBit = 1 << (2 * static_cast<int>(color));
            uint32_t qscBit = 2 << (2 * static_cast<int>(color));

            if ((attacks::kscBlockSquares<color>() & board.allPieces()).empty() && (board.castlingRights().value() & kscBit))
            {
                moves.push_back(Move(kingSq, kingSq + 2, MoveType::CASTLE));
            }

            if ((attacks::qscBlockSquares<color>() & board.allPieces()).empty() && (board.castlingRights().value() & qscBit))
            {
                moves.push_back(Move(kingSq, kingSq - 2, MoveType::CASTLE));
            }
        }
    }
}

template<MoveGenType type, Color color, PieceType piece>
void genPieceMoves(const Board& board, MoveList& moves, Bitboard moveMask)
{
    Bitboard pieceBB = board.pieces(color, piece);
    Bitboard allPieces = board.allPieces();

    while (pieceBB.any())
    {
        Square from = pieceBB.poplsb();
        Bitboard attacks = attacks::pieceAttacks<piece>(from, allPieces) & moveMask;
        while (attacks.any())
        {
            Square to = attacks.poplsb();
            moves.push_back(Move(from, to, MoveType::NONE));
        }
    }
}

template<MoveGenType type, Color color>
void genPawnMoves(const Board& board, MoveList& moves, Bitboard moveMask)
{
    Bitboard pawns = board.pieces(color, PieceType::PAWN);
    Bitboard allPieces = board.allPieces();
    Bitboard oppBB = board.pieces(~color);

    if constexpr (type == MoveGenType::NOISY_QUIET)
    {
        Bitboard pawnPushes = attacks::pawnPushes<color>(pawns);
        pawnPushes &= ~allPieces;

        Bitboard doublePushes = attacks::pawnPushes<color>(pawnPushes & Bitboard::nthRank<color, RANK_3>());
        doublePushes &= ~allPieces;
        doublePushes &= moveMask;

        pawnPushes &= moveMask;

        Bitboard promotions = pawnPushes & Bitboard::nthRank<color, RANK_8>();

        pawnPushes ^= promotions;

        while (pawnPushes.any())
        {
            Square push = pawnPushes.poplsb();
            moves.push_back(Move(push - attacks::pawnPushOffset<color>(), push, MoveType::NONE));
        }

        while (doublePushes.any())
        {
            Square dPush = doublePushes.poplsb();
            moves.push_back(Move(dPush - 2 * attacks::pawnPushOffset<color>(), dPush, MoveType::NONE));
        }

        while (promotions.any())
        {
            Square promotion = promotions.poplsb();
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

        Bitboard promotions = pawnPushes & Bitboard::nthRank<color, RANK_8>();

        while (promotions.any())
        {
            Square promotion = promotions.poplsb();
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(promotion - attacks::pawnPushOffset<color>(), promotion, MoveType::PROMOTION, Promotion::KNIGHT));
        }
    }
    else if constexpr (type == MoveGenType::QUIET)
    {
        Bitboard pawnPushes = attacks::pawnPushes<color>(pawns);
        pawnPushes &= ~allPieces;

        Bitboard doublePushes = attacks::pawnPushes<color>(pawnPushes & Bitboard::nthRank<color, 2>());
        doublePushes &= ~allPieces;
        doublePushes &= moveMask;

        pawnPushes &= moveMask;

        Bitboard promotions = pawnPushes & Bitboard::nthRank<color, 7>();
        pawnPushes ^= promotions;

        while (pawnPushes.any())
        {
            Square push = pawnPushes.poplsb();
            moves.push_back(Move(push - attacks::pawnPushOffset<color>(), push, MoveType::NONE));
        }

        while (doublePushes.any())
        {
            Square dPush = doublePushes.poplsb();
            moves.push_back(Move(dPush - 2 * attacks::pawnPushOffset<color>(), dPush, MoveType::NONE));
        }

        // rest of the pawn movegen is captures/promos
        return;
    }



    Bitboard eastCaptures = attacks::pawnEastAttacks<color>(pawns);
    if (board.epSquare() != -1 && (eastCaptures & Bitboard::fromSquare(Square(board.epSquare()))).any() && (Bitboard::fromSquare(Square(board.epSquare()) - attacks::pawnPushOffset<color>()) & moveMask).any())
        moves.push_back(Move(Square(board.epSquare()) - attacks::pawnPushOffset<color>() - 1, Square(board.epSquare()), MoveType::ENPASSANT));

    eastCaptures &= oppBB;
    eastCaptures &= moveMask;

    Bitboard promotions = eastCaptures & Bitboard::nthRank<color, RANK_8>();
    eastCaptures ^= promotions;

    while (eastCaptures.any())
    {
        Square capture = eastCaptures.poplsb();
        moves.push_back(Move(capture - attacks::pawnPushOffset<color>() - 1, capture, MoveType::NONE));
    }

    while (promotions.any())
    {
        Square promotion = promotions.poplsb();
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() - 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }


    Bitboard westCaptures = attacks::pawnWestAttacks<color>(pawns);
    if (board.epSquare() != -1 && (westCaptures & Bitboard::fromSquare(Square(board.epSquare()))).any() && (Bitboard::fromSquare(Square(board.epSquare()) - attacks::pawnPushOffset<color>()) & moveMask).any())
        moves.push_back(Move(Square(board.epSquare()) - attacks::pawnPushOffset<color>() + 1, Square(board.epSquare()), MoveType::ENPASSANT));

    westCaptures &= oppBB;
    westCaptures &= moveMask;

    promotions = westCaptures & Bitboard::nthRank<color, RANK_8>();
    westCaptures ^= promotions;

    while (westCaptures.any())
    {
        Square capture = westCaptures.poplsb();
        moves.push_back(Move(capture - attacks::pawnPushOffset<color>() + 1, capture, MoveType::NONE));
    }

    while (promotions.any())
    {
        Square promotion = promotions.poplsb();
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(promotion - attacks::pawnPushOffset<color>() + 1, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }
}
