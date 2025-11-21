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
    usize j = 0;
    for (usize i = 0; i < moves.size(); i++)
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
        Bitboard moveMask = ~board.pieces(color)
            & (checkers.any() ? attacks::moveMask(board.kingSq(color), checkers.lsb()) : ALL_BB);
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
            CastlingRights kingSide(color, CastleSide::KING_SIDE);
            CastlingRights queenSide(color, CastleSide::QUEEN_SIDE);

            Square kingSideRook = board.castlingRookSq(color, CastleSide::KING_SIDE);
            Square queenSideRook = board.castlingRookSq(color, CastleSide::QUEEN_SIDE);

            if (board.castlingRights().has(kingSide)
                && !board.castlingBlocked(color, CastleSide::KING_SIDE))
            {
                moves.push_back(Move(kingSq, kingSideRook, MoveType::CASTLE));
            }

            if (board.castlingRights().has(queenSide)
                && !board.castlingBlocked(color, CastleSide::QUEEN_SIDE))
            {
                moves.push_back(Move(kingSq, queenSideRook, MoveType::CASTLE));
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
    constexpr i32 PUSH_OFFSET = attacks::pawnPushOffset<color>();

    Bitboard pawns = board.pieces(color, PieceType::PAWN);
    Bitboard allPieces = board.allPieces();
    Bitboard oppBB = board.pieces(~color);

    if constexpr (type == MoveGenType::NOISY_QUIET)
    {
        Bitboard pawnPushes = attacks::pawnPushes<color>(pawns);
        pawnPushes &= ~allPieces;

        Bitboard doublePushes =
            attacks::pawnPushes<color>(pawnPushes & Bitboard::nthRank<color, RANK_3>());
        doublePushes &= ~allPieces;
        doublePushes &= moveMask;

        pawnPushes &= moveMask;

        Bitboard promotions = pawnPushes & Bitboard::nthRank<color, RANK_8>();

        pawnPushes ^= promotions;

        while (pawnPushes.any())
        {
            Square push = pawnPushes.poplsb();
            Square from = push - PUSH_OFFSET;
            moves.push_back(Move(from, push, MoveType::NONE));
        }

        while (doublePushes.any())
        {
            Square dPush = doublePushes.poplsb();
            Square from = dPush - 2 * PUSH_OFFSET;
            moves.push_back(Move(from, dPush, MoveType::NONE));
        }

        while (promotions.any())
        {
            Square promotion = promotions.poplsb();
            Square from = promotion - PUSH_OFFSET;
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
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
            Square from = promotion - PUSH_OFFSET;
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::QUEEN));
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::ROOK));
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::BISHOP));
            moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
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
            Square from = push - PUSH_OFFSET;
            moves.push_back(Move(from, push, MoveType::NONE));
        }

        while (doublePushes.any())
        {
            Square dPush = doublePushes.poplsb();
            Square from = dPush - 2 * PUSH_OFFSET;
            moves.push_back(Move(from, dPush, MoveType::NONE));
        }

        // rest of the pawn movegen is captures/promos
        return;
    }

    Bitboard eastCaptures = attacks::pawnEastAttacks<color>(pawns);
    if (board.epSquare() != -1 && eastCaptures.has(Square(board.epSquare()))
        && moveMask.has(Square(board.epSquare()) - PUSH_OFFSET))
    {
        Square to = Square(board.epSquare());
        Square from = to - PUSH_OFFSET - 1;
        moves.push_back(Move(from, to, MoveType::ENPASSANT));
    }

    eastCaptures &= oppBB;
    eastCaptures &= moveMask;

    Bitboard promotions = eastCaptures & Bitboard::nthRank<color, RANK_8>();
    eastCaptures ^= promotions;

    while (eastCaptures.any())
    {
        Square capture = eastCaptures.poplsb();
        Square from = capture - PUSH_OFFSET - 1;
        moves.push_back(Move(from, capture, MoveType::NONE));
    }

    while (promotions.any())
    {
        Square promotion = promotions.poplsb();
        Square from = promotion - PUSH_OFFSET - 1;
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }

    Bitboard westCaptures = attacks::pawnWestAttacks<color>(pawns);
    if (board.epSquare() != -1 && westCaptures.has(Square(board.epSquare()))
        && moveMask.has(Square(board.epSquare()) - PUSH_OFFSET))
    {
        Square to = Square(board.epSquare());
        Square from = to - PUSH_OFFSET + 1;
        moves.push_back(Move(from, to, MoveType::ENPASSANT));
    }

    westCaptures &= oppBB;
    westCaptures &= moveMask;

    promotions = westCaptures & Bitboard::nthRank<color, RANK_8>();
    westCaptures ^= promotions;

    while (westCaptures.any())
    {
        Square capture = westCaptures.poplsb();
        Square from = capture - PUSH_OFFSET + 1;
        moves.push_back(Move(from, capture, MoveType::NONE));
    }

    while (promotions.any())
    {
        Square promotion = promotions.poplsb();
        Square from = promotion - PUSH_OFFSET + 1;
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::QUEEN));
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::ROOK));
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::BISHOP));
        moves.push_back(Move(from, promotion, MoveType::PROMOTION, Promotion::KNIGHT));
    }
}
