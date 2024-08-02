#include "move.h"

namespace comm
{

const char promoChars[4] = {'N', 'B', 'R', 'Q'};
const char pieceChars[5] = {'N', 'B', 'R', 'Q', 'K'};

MoveStrFind findMoveFromPCN(const MoveList& legalMoves, const char* moveStr)
{
    int src = (moveStr[0] - 'a') + ((moveStr[1] - '1') << 3);
    int dst = (moveStr[2] - 'a') + ((moveStr[3] - '1') << 3);

    Promotion promotion = Promotion(-1);
    bool isPromotion = false;
    switch (tolower(moveStr[4]))
    {
        case 'q':
            promotion = Promotion::QUEEN;
            isPromotion = true;
            break;
        case 'r':
            promotion = Promotion::ROOK;
            isPromotion = true;
            break;
        case 'b':
            promotion = Promotion::BISHOP;
            isPromotion = true;
            break;
        case 'n':
            promotion = Promotion::KNIGHT;
            isPromotion = true;
            break;
    }

    for (size_t i = 0; i < legalMoves.size(); i++)
    {
        Move move = legalMoves[i];
        if (move.fromSq() == Square(src) && move.toSq() == Square(dst))
        {
            if (isPromotion)
            {
                if (move.type() == MoveType::PROMOTION && move.promotion() == promotion)
                    return {MoveStrFind::Result::FOUND, move, 5};
            }
            else
            {
                return {MoveStrFind::Result::FOUND, move, 4};
            }
        }
    }
    return {MoveStrFind::Result::NOT_FOUND, Move(), 4 + isPromotion};
}

MoveStrFind findMoveFromSAN(const Board& board, const MoveList& legalMoves, const char* moveStr)
{
    int fromFile = -1;
    int fromRank = -1;
    int toFile = -1;
    int toRank = -1;
    PieceType piece = PieceType::NONE;
    Promotion promotion = Promotion(-1);
    bool isPromotion = false;
    bool isCapture = false;

    int moveLen = -1;

    if (moveStr[0] == '\0')
        return {MoveStrFind::Result::INVALID, Move(), 0};

    switch (moveStr[0])
    {
        case 'K':
            piece = PieceType::KING;
            goto piece_moves;
        case 'Q':
            piece = PieceType::QUEEN;
            goto piece_moves;
        case 'R':
            piece = PieceType::ROOK;
            goto piece_moves;
        case 'B':
            piece = PieceType::BISHOP;
            goto piece_moves;
        case 'N':
            piece = PieceType::KNIGHT;
            goto piece_moves;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
            piece = PieceType::PAWN;
            if (moveStr[1] == 'x')
            {
                isCapture = true;
                fromFile = moveStr[0] - 'a';
            }
            else
            {
                toFile = moveStr[0] - 'a';
            }
            break;
        case 'O':
            if (moveStr[1] != '-')
                return {MoveStrFind::Result::INVALID, Move(), 0};
            if (moveStr[2] != 'O')
                return {MoveStrFind::Result::INVALID, Move(), 0};
            if (moveStr[3] != '-')
            {
                for (size_t i = 0; i < legalMoves.size(); i++)
                {
                    Move move = legalMoves[i];
                    if (move.type() == MoveType::CASTLE && move.toSq() > move.fromSq())
                    {
                        return {MoveStrFind::Result::FOUND, move, 3 + (moveStr[3] == '+' || moveStr[3] == '#')};
                    }
                }
                return {MoveStrFind::Result::NOT_FOUND, Move(), 3};
            }
            else
            {
                if (moveStr[4] != 'O')
                    return {MoveStrFind::Result::INVALID, Move(), 0};
                for (size_t i = 0; i < legalMoves.size(); i++)
                {
                    Move move = legalMoves[i];
                    if (move.type() == MoveType::CASTLE && move.toSq() < move.fromSq())
                    {
                        return {MoveStrFind::Result::FOUND, move, 5 + (moveStr[5] == '+' || moveStr[5] == '#')};
                    }
                }
                return {MoveStrFind::Result::NOT_FOUND, Move(), 5};
            }
        default:
            return {MoveStrFind::Result::INVALID, Move(), 0};
    }

    if (isCapture)
    {
        if (moveStr[2] < 'a' || moveStr[2] > 'h' || moveStr[3] < '1' || moveStr[3] > '8')
            return {MoveStrFind::Result::INVALID, Move(), 0};
        toFile = moveStr[2] - 'a';
        toRank = moveStr[3] - '1';
        int i = 4;
        if (moveStr[4] == '=')
            i = 5;
        switch (moveStr[i])
        {
            case 'Q':
                promotion = Promotion::QUEEN;
                isPromotion = true;
                break;
            case 'R':
                promotion = Promotion::ROOK;
                isPromotion = true;
                break;
            case 'B':
                promotion = Promotion::BISHOP;
                isPromotion = true;
                break;
            case 'N':
                promotion = Promotion::KNIGHT;
                isPromotion = true;
                break;
        }

        moveLen = i + isPromotion;
    }
    else
    {
        if (moveStr[1] < '1' || moveStr[1] > '8')
            return {MoveStrFind::Result::INVALID, Move(), 0};
        toRank = moveStr[1] - '1';
        int i = 2;
        if (moveStr[i] == '=')
            i = 3;

        switch (moveStr[i])
        {
            case 'Q':
                promotion = Promotion::QUEEN;
                isPromotion = true;
                break;
            case 'R':
                promotion = Promotion::ROOK;
                isPromotion = true;
                break;
            case 'B':
                promotion = Promotion::BISHOP;
                isPromotion = true;
                break;
            case 'N':
                promotion = Promotion::KNIGHT;
                isPromotion = true;
                break;
        }
        moveLen = i + isPromotion;
    }
    goto search_moves;
piece_moves:
    if (moveStr[1] == '\0')
        return {MoveStrFind::Result::INVALID, Move(), 0};
    if (moveStr[2] == '\0')
        return {MoveStrFind::Result::INVALID, Move(), 0};
    if (moveStr[1] == 'x')
    {
        isCapture = true;
        if (moveStr[2] < 'a' || moveStr[2] > 'h' || moveStr[3] < '1' || moveStr[3] > '8')
            return {MoveStrFind::Result::INVALID, Move(), 0};
        toFile = moveStr[2] - 'a';
        toRank = moveStr[3] - '1';
        moveLen = 4;
    }
    else if (moveStr[2] == 'x')
    {
        isCapture = true;
        switch (moveStr[1])
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
                fromFile = moveStr[1] - 'a';
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                fromRank = moveStr[1] - '1';
                break;
            default:
                return {MoveStrFind::Result::INVALID, Move(), 0};
        }
        if (moveStr[3] < 'a' || moveStr[3] > 'h' || moveStr[4] < '1' || moveStr[4] > '8')
            return {MoveStrFind::Result::INVALID, Move(), 0};
        toFile = moveStr[3] - 'a';
        toRank = moveStr[4] - '1';
        moveLen = 5;
    }
    else if (moveStr[3] == 'x')
    {
        if (moveStr[4] < 'a' || moveStr[4] > 'h' || moveStr[5] < '1' || moveStr[5] > '8')
            return {MoveStrFind::Result::INVALID, Move(), 0};

        isCapture = true;
        fromFile = moveStr[1] - 'a';
        fromRank = moveStr[2] - '1';

        toFile = moveStr[4] - 'a';
        toRank = moveStr[5] - '1';
        moveLen = 6;
    }
    else
    {
        int file1 = -1;
        int rank1 = -1;
        switch (moveStr[1])
        {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
                file1 = moveStr[1] - 'a';
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                rank1 = moveStr[1] - '1';
                break;
            default:
                return {MoveStrFind::Result::INVALID, Move(), 0};
        }

        if (file1 != -1)
        {
            switch (moveStr[2])
            {
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                    toFile = moveStr[2] - 'a';
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    rank1 = moveStr[2] - '1';
                    break;
                default:
                    return {MoveStrFind::Result::INVALID, Move(), 0};
            }

            if (toFile != -1)
            {
                if (moveStr[3] < '1' || moveStr[3] > '8')
                    return {MoveStrFind::Result::INVALID, Move(), 0};
                fromFile = file1;
                toRank = moveStr[3] - '1';
                moveLen = 4;
            }
            else
            {
                if (moveStr[3] >= 'a' && moveStr[3] <= 'h')
                {
                    if (moveStr[4] < '1' || moveStr[4] > '8')
                        return {MoveStrFind::Result::INVALID, Move(), 0};
                    fromFile = file1;
                    fromRank = rank1;

                    toFile = moveStr[3] - 'a';
                    toRank = moveStr[4] - '1';
                    moveLen = 5;
                }
                else
                {
                    toFile = file1;
                    toRank = rank1;
                    moveLen = 3;
                }
            }
        }
        else
        {
            fromRank = rank1;
            if (moveStr[2] < 'a' || moveStr[2] > 'h' || moveStr[3] < '1' || moveStr[3] > '8')
                return {MoveStrFind::Result::INVALID, Move(), 0};
            toFile = moveStr[2] - 'a';
            toRank = moveStr[3] - '1';
            moveLen = 4;
        }
    }
search_moves:
    if (moveStr[moveLen] == '+')
        moveLen++;
    else if (moveStr[moveLen] == '#')
        moveLen++;
    Square toSquare(toRank, toFile);

    if (!isCapture && board.pieceAt(toSquare) != Piece::NONE)
    {
        return {MoveStrFind::Result::NOT_FOUND, Move(), moveLen};
    }

    int pawnOffset = (board.sideToMove() == Color::WHITE) ? -8 : 8;

    const Move* match = nullptr;
    for (const Move* it = legalMoves.data(); it != legalMoves.data() + legalMoves.size(); it++)
    {
        switch (it->type())
        {
            case MoveType::ENPASSANT:
                if (isCapture && (toRank == 2 || toRank == 5) && board.pieceAt(toSquare + pawnOffset) == Piece::NONE)
                    continue;
                break;
            default:
                if (isCapture && board.pieceAt(toSquare) == Piece::NONE)
                    continue;
                break;
        }
        if (isPromotion && it->type() != MoveType::PROMOTION)
            continue;
        if (!isPromotion && it->type() == MoveType::PROMOTION)
            continue;

        if (isPromotion && it->promotion() != promotion)
            continue;

        if (it->toSq() != Square(toSquare))
            continue;

        Square srcSquare = it->fromSq();
        Piece srcPiece = board.pieceAt(srcSquare);
        if (getPieceType(srcPiece) != piece)
            continue;

        if (fromRank != -1 && fromRank != (srcSquare.rank()))
            continue;
        if (fromFile != -1 && fromFile != (srcSquare.file()))
            continue;

        if (match)
            return {MoveStrFind::Result::AMBIGUOUS, Move(), moveLen};
        match = it;
    }

    if (match)
        return {MoveStrFind::Result::FOUND, *match, moveLen};
    else
        return {MoveStrFind::Result::NOT_FOUND, Move(), moveLen};
}


std::string convMoveToPCN(Move move)
{
    std::string str(4 + (move.type() == MoveType::PROMOTION), ' ');
    str[0] = static_cast<char>(move.fromSq().file() + 'a');
    str[1] = static_cast<char>(move.fromSq().rank() + '1');
    str[2] = static_cast<char>(move.toSq().file() + 'a');
    str[3] = static_cast<char>(move.toSq().rank() + '1');
    if (move.type() == MoveType::PROMOTION)
    {
        str[4] = promoChars[(static_cast<int>(move.promotion()) >> 14)];
    }
    return str;
}

bool isAmbiguous(const Board& board, const MoveList& legalMoves, PieceType piece, Square toSquare, int fromFile, int fromRank)
{
    const Move* match = nullptr;
    for (const Move* it = legalMoves.data(); it != legalMoves.data() + legalMoves.size(); it++)
    {
        Square srcPos = it->fromSq();
        Square dstPos = it->toSq();
        PieceType movePiece = getPieceType(board.pieceAt(srcPos));
        if (movePiece != piece)
            continue;

        if (dstPos != toSquare)
            continue;

        if (fromFile != -1 && fromFile != srcPos.file())
            continue;

        if (fromRank != -1 && fromRank != srcPos.rank())
            continue;

        if (match)
            return true;
        else
            match = it;
    }
    return false;
}

std::string convMoveToSAN(const Board& board, const MoveList& legalMoves, Move move)
{
    if (move.type() == MoveType::CASTLE)
    {
        return move.toSq() > move.fromSq() ? "O-O" : "O-O-O";
    }
    PieceType piece = getPieceType(board.pieceAt(move.fromSq()));
    bool isCapture = board.pieceAt(move.toSq()) != Piece::NONE;
    if (piece == PieceType::PAWN)
    {
        Square srcPos = move.fromSq();
        Square dstPos = move.toSq();
        if (srcPos.file() == dstPos.file())
        {
            std::string str(2 + 2 * (move.type() == MoveType::PROMOTION), ' ');
            if (move.type() == MoveType::PROMOTION)
            {
                str[2] = '=';
                str[3] = promoChars[static_cast<int>(move.promotion()) >> 14];
            }
            str[0] = static_cast<char>(dstPos.file() + 'a');
            str[1] = static_cast<char>(dstPos.rank() + '1');
            return str;
        }
        else
        {
            std::string str(4 + 2 * (move.type() == MoveType::PROMOTION), ' ');
            if (move.type() == MoveType::PROMOTION)
            {
                str[4] = '=';
                str[5] = promoChars[static_cast<int>(move.promotion()) >> 14];
            }
            str[0] = static_cast<char>(srcPos.file() + 'a');
            str[1] = 'x';
            str[2] = static_cast<char>(dstPos.file() + 'a');
            str[3] = static_cast<char>(dstPos.rank() + '1');
            return str;
        }
    }
    else
    {
        char pceChar = pieceChars[static_cast<int>(piece) - static_cast<int>(PieceType::KNIGHT)];

        Square srcPos = move.fromSq();
        Square dstPos = move.toSq();

        if (!isAmbiguous(board, legalMoves, piece, dstPos, -1, -1))
        {
            std::string str(3 + isCapture, ' ');
            str[0] = pceChar;
            if (isCapture)
            {
                str[1] = 'x';
            }
            str[1 + isCapture] = static_cast<char>(dstPos.file() + 'a');
            str[2 + isCapture] = static_cast<char>(dstPos.rank() + '1');
            return str;
        }

        if (!isAmbiguous(board, legalMoves, piece, dstPos, srcPos.file(), -1))
        {
            std::string str(4 + isCapture, ' ');
            str[0] = pceChar;
            str[1] = static_cast<char>(srcPos.file() + 'a');
            if (isCapture)
            {
                str[2] = 'x';
            }
            str[2 + isCapture] = static_cast<char>(dstPos.file() + 'a');
            str[3 + isCapture] = static_cast<char>(dstPos.rank() + '1');
            return str;
        }

        if (!isAmbiguous(board, legalMoves, piece, dstPos, srcPos.rank(), -1))
        {
            std::string str(4 + isCapture, ' ');
            str[0] = pceChar;
            str[1] = static_cast<char>(srcPos.rank() + '1');
            if (isCapture)
            {
                str[2] = 'x';
            }
            str[2 + isCapture] = static_cast<char>(dstPos.file() + 'a');
            str[3 + isCapture] = static_cast<char>(dstPos.rank() + '1');
            return str;
        }

        std::string str(5 + isCapture, ' ');
        str[0] = pceChar;
        str[1] = static_cast<char>(srcPos.file() + 'a');
        str[2] = static_cast<char>(srcPos.rank() + '1');
        if (isCapture)
        {
            str[3] = 'x';
        }
        str[3 + isCapture] = static_cast<char>(dstPos.file() + 'a');
        str[4 + isCapture] = static_cast<char>(dstPos.rank() + '1');
        return str;
    }
}


}
