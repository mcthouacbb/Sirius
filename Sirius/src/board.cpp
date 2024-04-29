#include "board.h"
#include "attacks.h"

#include <cstring>
#include <charconv>

Board::Board()
{
    setToFen(defaultFen);
}

void Board::setToFen(const std::string_view& fen)
{
    m_States.clear();
    m_States.push_back(BoardState{});
    currState().pliesFromNull = 0;
    currState().repetitions = 0;
    currState().lastRepetition = 0;


    currState().squares.fill(Piece::NONE);
    currState().pieces = {};
    currState().colors = {};

    currState().evalState.init();
    currState().zkey.value = 0;
    currState().pawnKey.value = 0;

    int i = 0;
    int sq = 56;
    for (;; i++)
    {
        switch (fen[i])
        {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                sq += fen[i] - '0';
                break;
            case 'k':
                currState().zkey.addPiece(PieceType::KING, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::KING);
                break;
            case 'q':
                currState().zkey.addPiece(PieceType::QUEEN, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::QUEEN);
                break;
            case 'r':
                currState().zkey.addPiece(PieceType::ROOK, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::ROOK);
                break;
            case 'b':
                currState().zkey.addPiece(PieceType::BISHOP, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::BISHOP);
                break;
            case 'n':
                currState().zkey.addPiece(PieceType::KNIGHT, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
                break;
            case 'p':
                currState().zkey.addPiece(PieceType::PAWN, Color::BLACK, sq);
                currState().pawnKey.addPiece(PieceType::PAWN, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::PAWN);
                break;
            case 'K':
                currState().zkey.addPiece(PieceType::KING, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::KING);
                break;
            case 'Q':
                currState().zkey.addPiece(PieceType::QUEEN, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::QUEEN);
                break;
            case 'R':
                currState().zkey.addPiece(PieceType::ROOK, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::ROOK);
                break;
            case 'B':
                currState().zkey.addPiece(PieceType::BISHOP, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::BISHOP);
                break;
            case 'N':
                currState().zkey.addPiece(PieceType::KNIGHT, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
                break;
            case 'P':
                currState().zkey.addPiece(PieceType::PAWN, Color::WHITE, sq);
                currState().pawnKey.addPiece(PieceType::PAWN, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::PAWN);
                break;
            case '/':
                sq -= 16;
                break;
            default:
                goto done;
        }
    }
done:
    i++;
    m_SideToMove = fen[i] == 'w' ? Color::WHITE : Color::BLACK;
    if (m_SideToMove == Color::BLACK)
    {
        currState().zkey.flipSideToMove();
    }

    i += 2;
    currState().castlingRights = 0;
    if (fen[i] == '-')
    {
        i++;
    }
    else
    {
        if (fen[i] == 'K')
        {
            i++;
            currState().castlingRights |= 1;
        }

        if (fen[i] == 'Q')
        {
            i++;
            currState().castlingRights |= 2;
        }

        if (fen[i] == 'k')
        {
            i++;
            currState().castlingRights |= 4;
        }

        if (fen[i] == 'q')
        {
            i++;
            currState().castlingRights |= 8;
        }
    }

    currState().zkey.updateCastlingRights(currState().castlingRights);

    i++;

    if (fen[i] != '-')
    {
        currState().epSquare = fen[i] - 'a';
        currState().epSquare |= (fen[++i] - '1') << 3;
        currState().zkey.updateEP(currState().epSquare & 7);
    }
    else
    {
        currState().epSquare = -1;
    }
    i += 2;

    auto [ptr, ec] = std::from_chars(&fen[i], fen.data() + fen.size(), currState().halfMoveClock);
    std::from_chars(ptr + 1, fen.data() + fen.size(), m_GamePly);
    m_GamePly = 2 * m_GamePly - 1 - (m_SideToMove == Color::WHITE);

    updateCheckInfo();
}

void Board::setToEpd(const std::string_view& epd)
{
    m_States.clear();
    m_States.push_back(BoardState{});

    currState().pliesFromNull = 0;
    currState().repetitions = 0;
    currState().lastRepetition = 0;

    currState().squares.fill(Piece::NONE);
    currState().pieces = {};
    currState().colors = {};

    currState().evalState.init();
    currState().zkey.value = 0;
    currState().pawnKey.value = 0;

    int i = 0;
    int sq = 56;
    for (;; i++)
    {
        switch (epd[i])
        {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                sq += epd[i] - '0';
                break;
            case 'k':
                currState().zkey.addPiece(PieceType::KING, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::KING);
                break;
            case 'q':
                currState().zkey.addPiece(PieceType::QUEEN, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::QUEEN);
                break;
            case 'r':
                currState().zkey.addPiece(PieceType::ROOK, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::ROOK);
                break;
            case 'b':
                currState().zkey.addPiece(PieceType::BISHOP, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::BISHOP);
                break;
            case 'n':
                currState().zkey.addPiece(PieceType::KNIGHT, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
                break;
            case 'p':
                currState().zkey.addPiece(PieceType::PAWN, Color::BLACK, sq);
                currState().pawnKey.addPiece(PieceType::PAWN, Color::BLACK, sq);
                addPiece(sq++, Color::BLACK, PieceType::PAWN);
                break;
            case 'K':
                currState().zkey.addPiece(PieceType::KING, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::KING);
                break;
            case 'Q':
                currState().zkey.addPiece(PieceType::QUEEN, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::QUEEN);
                break;
            case 'R':
                currState().zkey.addPiece(PieceType::ROOK, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::ROOK);
                break;
            case 'B':
                currState().zkey.addPiece(PieceType::BISHOP, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::BISHOP);
                break;
            case 'N':
                currState().zkey.addPiece(PieceType::KNIGHT, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
                break;
            case 'P':
                currState().zkey.addPiece(PieceType::PAWN, Color::WHITE, sq);
                currState().pawnKey.addPiece(PieceType::PAWN, Color::WHITE, sq);
                addPiece(sq++, Color::WHITE, PieceType::PAWN);
                break;
            case '/':
                sq -= 16;
                break;
            default:
                goto done;
        }
    }
done:
    i++;
    m_SideToMove = epd[i] == 'w' ? Color::WHITE : Color::BLACK;
    if (m_SideToMove == Color::BLACK)
    {
        currState().zkey.flipSideToMove();
    }

    i += 2;
    currState().castlingRights = 0;
    if (epd[i] == '-')
    {
        i++;
    }
    else
    {
        if (epd[i] == 'K')
        {
            i++;
            currState().castlingRights |= 1;
        }

        if (epd[i] == 'Q')
        {
            i++;
            currState().castlingRights |= 2;
        }

        if (epd[i] == 'k')
        {
            i++;
            currState().castlingRights |= 4;
        }

        if (epd[i] == 'q')
        {
            i++;
            currState().castlingRights |= 8;
        }
    }

    currState().zkey.updateCastlingRights(currState().castlingRights);

    i++;

    if (epd[i] != '-')
    {
        currState().epSquare = epd[i] - 'a';
        currState().epSquare |= (epd[++i] - '1') << 3;
        currState().zkey.updateEP(currState().epSquare & 7);
    }
    else
    {
        currState().epSquare = -1;
    }
    i += 2;

    currState().halfMoveClock = 0;
    m_GamePly = 0;

    updateCheckInfo();
}

constexpr std::array<char, 16> pieceChars = {
    'P', 'N', 'B', 'R', 'Q', 'K', ' ', ' ',
    'p', 'n', 'b', 'r', 'q', 'k', ' ', ' '
};

std::string Board::stringRep() const
{

    std::string result;
    const char* between = "+---+---+---+---+---+---+---+---+\n";

    for (int j = 56; j >= 0; j -= 8)
    {
        result += between;
        for (int i = j; i < j + 8; i++)
        {
            result += "| ";
            Piece piece = currState().squares[i];
            result += pieceChars[static_cast<int>(piece)];
            result += " ";
        }
        result += "|\n";
    }
    result += between;
    return result;
}

std::string Board::fenStr() const
{
    std::string fen = "";
    int lastFile;
    for (int j = 56; j >= 0; j -= 8)
    {
        lastFile = -1;
        for (int i = j; i < j + 8; i++)
        {
            Piece piece = currState().squares[i];
            if (piece != Piece::NONE)
            {
                int diff = i - j - lastFile;
                if (diff > 1)
                    fen += static_cast<char>((diff - 1) + '0');
                fen += pieceChars[static_cast<int>(piece)];
                lastFile = i - j;
            }
        }
        int diff = 8 - lastFile;
        if (diff > 1)
            fen += static_cast<char>((diff - 1) + '0');
        if (j != 0)
            fen += '/';
    }

    fen += ' ';

    fen += m_SideToMove == Color::WHITE ? "w " : "b ";

    if (currState().castlingRights == 0)
        fen += '-';
    else
    {
        if (currState().castlingRights & 1)
            fen += 'K';
        if (currState().castlingRights & 2)
            fen += 'Q';
        if (currState().castlingRights & 4)
            fen += 'k';
        if (currState().castlingRights & 8)
            fen += 'q';
    }

    fen += ' ';

    if (currState().epSquare == -1)
        fen += '-';
    else
    {
        fen += static_cast<char>((currState().epSquare & 7) + 'a');
        fen += static_cast<char>((currState().epSquare >> 3) + '1');
    }

    fen += ' ';
    fen += std::to_string(currState().halfMoveClock);
    fen += ' ';
    fen += std::to_string(m_GamePly / 2 + (m_SideToMove == Color::BLACK));

    return fen;
}

std::string Board::epdStr() const
{
    std::string epd = "";
    int lastFile;
    for (int j = 56; j >= 0; j -= 8)
    {
        lastFile = -1;
        for (int i = j; i < j + 8; i++)
        {
            Piece piece = currState().squares[i];
            if (piece != Piece::NONE)
            {
                int diff = i - j - lastFile;
                if (diff > 1)
                    epd += static_cast<char>((diff - 1) + '0');
                epd += pieceChars[static_cast<int>(piece)];
                lastFile = i - j;
            }
        }
        int diff = 8 - lastFile;
        if (diff > 1)
            epd += static_cast<char>((diff - 1) + '0');
        if (j != 0)
            epd += '/';
    }

    epd += ' ';

    epd += m_SideToMove == Color::WHITE ? "w " : "b ";

    if (currState().castlingRights == 0)
        epd += '-';
    else
    {
        if (currState().castlingRights & 1)
            epd += 'K';
        if (currState().castlingRights & 2)
            epd += 'Q';
        if (currState().castlingRights & 4)
            epd += 'k';
        if (currState().castlingRights & 8)
            epd += 'q';
    }

    epd += ' ';

    if (currState().epSquare == -1)
        epd += '-';
    else
    {
        epd += static_cast<char>((currState().epSquare & 7) + 'a');
        epd += static_cast<char>((currState().epSquare >> 3) + '1');
    }

    return epd;
}

void Board::makeMove(Move move)
{
    m_States.push_back(currState());
    BoardState& prev = m_States[m_States.size() - 2];

    currState().halfMoveClock = prev.halfMoveClock + 1;
    currState().pliesFromNull = prev.pliesFromNull + 1;
    currState().epSquare = prev.epSquare;
    currState().castlingRights = prev.castlingRights;
    currState().zkey = prev.zkey;
    currState().pawnKey = prev.pawnKey;

    m_GamePly++;

    currState().zkey.flipSideToMove();

    if (currState().epSquare != -1)
        currState().zkey.updateEP(currState().epSquare & 7);

    switch (move.type())
    {
        case MoveType::NONE:
        {
            Piece srcPiece = currState().squares[move.srcPos()];

            Piece dstPiece = currState().squares[move.dstPos()];

            if (dstPiece != Piece::NONE)
            {
                currState().halfMoveClock = 0;
                removePiece(move.dstPos());
                currState().zkey.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());
                if (getPieceType(dstPiece) == PieceType::PAWN)
                    currState().pawnKey.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());
            }

            movePiece(move.srcPos(), move.dstPos());
            currState().zkey.movePiece(getPieceType(srcPiece), getPieceColor(srcPiece), move.srcPos(), move.dstPos());

            if (getPieceType(srcPiece) == PieceType::PAWN)
            {
                currState().pawnKey.movePiece(getPieceType(srcPiece), getPieceColor(srcPiece), move.srcPos(), move.dstPos());
                currState().halfMoveClock = 0;
                if (std::abs(move.srcPos() - move.dstPos()) == 16)
                    currState().epSquare = (move.srcPos() + move.dstPos()) / 2;
            }
            break;
        }
        case MoveType::PROMOTION:
        {
            currState().halfMoveClock = 0;

            Piece dstPiece = currState().squares[move.dstPos()];

            if (dstPiece != Piece::NONE)
            {
                removePiece(move.dstPos());
                currState().zkey.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());
            }
            removePiece(move.srcPos());
            currState().zkey.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
            currState().pawnKey.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
            addPiece(move.dstPos(), m_SideToMove, promoPiece(move.promotion()));
            currState().zkey.addPiece(promoPiece(move.promotion()), m_SideToMove, move.dstPos());
            break;
        }
        case MoveType::CASTLE:
        {
            if (move.srcPos() > move.dstPos())
            {
                // queen side
                movePiece(move.srcPos(), move.dstPos());
                currState().zkey.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
                movePiece(move.dstPos() - 2, move.srcPos() - 1);
                currState().zkey.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() - 2, move.srcPos() - 1);
            }
            else
            {
                // king side
                movePiece(move.srcPos(), move.dstPos());
                currState().zkey.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
                movePiece(move.dstPos() + 1, move.srcPos() + 1);
                currState().zkey.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() + 1, move.srcPos() + 1);
            }
            break;
        }
        case MoveType::ENPASSANT:
        {
            currState().halfMoveClock = 0;

            int offset = m_SideToMove == Color::WHITE ? -8 : 8;

            removePiece(move.dstPos() + offset);
            currState().zkey.removePiece(PieceType::PAWN, ~m_SideToMove, move.dstPos() + offset);
            currState().pawnKey.removePiece(PieceType::PAWN, ~m_SideToMove, move.dstPos() + offset);
            movePiece(move.srcPos(), move.dstPos());
            currState().zkey.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
            currState().pawnKey.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
            break;
        }
    }

    currState().zkey.updateCastlingRights(currState().castlingRights);

    currState().castlingRights &= attacks::castleRightsMask(move.srcPos());
    currState().castlingRights &= attacks::castleRightsMask(move.dstPos());

    currState().zkey.updateCastlingRights(currState().castlingRights);



    if (currState().epSquare == prev.epSquare)
        currState().epSquare = -1;

    if (currState().epSquare != -1)
        currState().zkey.updateEP(currState().epSquare & 7);

    m_SideToMove = ~m_SideToMove;

    updateCheckInfo();
    calcRepetitions();
}

void Board::unmakeMove()
{
    m_States.pop_back();
    m_GamePly--;

    m_SideToMove = ~m_SideToMove;
}

void Board::makeNullMove()
{
    m_States.push_back(currState());
    BoardState& prev = m_States[m_States.size() - 2];

    currState().halfMoveClock = prev.halfMoveClock + 1;
    currState().pliesFromNull = 0;
    currState().epSquare = -1;
    currState().castlingRights = prev.castlingRights;
    currState().zkey = prev.zkey;
    currState().pawnKey = prev.pawnKey;
    currState().repetitions = 0;
    currState().lastRepetition = 0;

    m_GamePly++;

    if (prev.epSquare != -1)
        currState().zkey.updateEP(prev.epSquare & 7);

    currState().zkey.flipSideToMove();
    m_SideToMove = ~m_SideToMove;

    updateCheckInfo();
}

void Board::unmakeNullMove()
{
    m_States.pop_back();

    m_GamePly--;

    m_SideToMove = ~m_SideToMove;
}

bool Board::squareAttacked(Color color, uint32_t square, Bitboard blockers) const
{
    return attackersTo(color, square, blockers).any();
}

Bitboard Board::attackersTo(Color color, uint32_t square, Bitboard blockers) const
{
    Bitboard queens = getPieces(PieceType::QUEEN);
    Bitboard pawns = (getPieces(color, PieceType::PAWN) & attacks::pawnAttacks(~color, square));
    Bitboard nonPawns =
        (getPieces(PieceType::KNIGHT) & attacks::knightAttacks(square)) |
        (getPieces(PieceType::KING) & attacks::kingAttacks(square)) |
        ((getPieces(PieceType::BISHOP) | queens) & attacks::bishopAttacks(square, blockers)) |
        ((getPieces(PieceType::ROOK) | queens) & attacks::rookAttacks(square, blockers));
    return pawns | (nonPawns & getColor(color));
}

Bitboard Board::attackersTo(uint32_t square, Bitboard blockers) const
{
    Bitboard queens = getPieces(PieceType::QUEEN);
    Bitboard pawns =
        (getPieces(Color::WHITE, PieceType::PAWN) & attacks::pawnAttacks(Color::BLACK, square)) |
        (getPieces(Color::BLACK, PieceType::PAWN) & attacks::pawnAttacks(Color::WHITE, square));

    Bitboard nonPawns =
        (getPieces(PieceType::KNIGHT) & attacks::knightAttacks(square)) |
        (getPieces(PieceType::KING) & attacks::kingAttacks(square)) |
        ((getPieces(PieceType::BISHOP) | queens) & attacks::bishopAttacks(square, blockers)) |
        ((getPieces(PieceType::ROOK) | queens) & attacks::rookAttacks(square, blockers));
    return pawns | nonPawns;
}

bool Board::isPassedPawn(uint32_t square) const
{
    Piece pce = getPieceAt(square);
    Bitboard mask = attacks::passedPawnMask(getPieceColor(pce), square);
    return (mask & getPieces(~getPieceColor(pce), PieceType::PAWN)).empty();
}

bool Board::isIsolatedPawn(uint32_t square) const
{
    Piece pce = getPieceAt(square);
    Bitboard mask = attacks::isolatedPawnMask(square);
    return (mask & getPieces(getPieceColor(pce), PieceType::PAWN)).empty();
}

Bitboard Board::pinnersBlockers(uint32_t square, Bitboard attackers, Bitboard& pinners) const
{
    Bitboard queens = getPieces(PieceType::QUEEN);
    attackers &=
        (attacks::rookAttacks(square, 0) & (getPieces(PieceType::ROOK) | queens)) |
        (attacks::bishopAttacks(square, 0) & (getPieces(PieceType::BISHOP) | queens));

    Bitboard blockers = 0;

    Bitboard blockMask = getAllPieces() ^ attackers;

    Bitboard sameColor = getColor(getPieceColor(currState().squares[square]));

    while (attackers)
    {
        uint32_t attacker = attackers.poplsb();

        Bitboard between = attacks::inBetweenSquares(square, attacker) & blockMask;

        if (between.one())
        {
            blockers |= between;
            if (between & sameColor)
                pinners |= Bitboard::fromSquare(attacker);
        }
    }
    return blockers;
}

// mostly from stockfish
bool Board::see(Move move, int margin) const
{
    int src = move.srcPos();
    int dst = move.dstPos();

    Bitboard allPieces = getAllPieces() ^ Bitboard::fromSquare(src);
    Bitboard attackers = attackersTo(dst, allPieces) ^ Bitboard::fromSquare(src);

    int value = 0;
    switch (move.type())
    {
        case MoveType::CASTLE:
            // rook and king cannot be attacked after castle
            return 0 >= margin;
        case MoveType::NONE:
        {
            PieceType type = getPieceType(getPieceAt(dst));
            if (type == PieceType::NONE)
                value = -margin;
            else
                value = seePieceValue(type) - margin;
            if (value < 0)
                return false;
            value = seePieceValue(getPieceType(getPieceAt(src))) - value;
            break;
        }
        case MoveType::PROMOTION:
        {
            PieceType promo = promoPiece(move.promotion());
            PieceType type = getPieceType(getPieceAt(dst));
            value = seePieceValue(promo) - seePieceValue(PieceType::PAWN);
            if (type == PieceType::NONE)
                value -= margin;
            else
                value += seePieceValue(type) - margin;
            if (value < 0)
                return false;

            value = seePieceValue(promo) - value;
            break;
        }
        case MoveType::ENPASSANT:
            value = seePieceValue(PieceType::PAWN) - margin;
            if (value < 0)
                return false;
            value = seePieceValue(PieceType::PAWN) - value;
            break;
    }

    if (value <= 0)
        return true;

    Color sideToMove = m_SideToMove;

    Bitboard diagPieces = getPieces(PieceType::BISHOP) | getPieces(PieceType::QUEEN);
    Bitboard straightPieces = getPieces(PieceType::ROOK) | getPieces(PieceType::QUEEN);

    bool us = false;

    while (true)
    {
        sideToMove = ~sideToMove;
        Bitboard stmAttackers = attackers & getColor(sideToMove);
        if (!stmAttackers)
            return !us;

        if (Bitboard pawns = (stmAttackers & getPieces(PieceType::PAWN)))
        {
            Bitboard pawn = pawns.lsbBB();
            allPieces ^= pawn;
            attackers ^= pawn;
            attackers |= (attacks::bishopAttacks(dst, allPieces) & allPieces & diagPieces);

            value = seePieceValue(PieceType::PAWN) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard knights = (stmAttackers & getPieces(PieceType::KNIGHT)))
        {
            Bitboard knight = knights.lsbBB();
            allPieces ^= knight;
            attackers ^= knight;

            value = seePieceValue(PieceType::KNIGHT) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard bishops = (stmAttackers & getPieces(PieceType::BISHOP)))
        {
            Bitboard bishop = bishops.lsbBB();
            allPieces ^= bishop;
            attackers ^= bishop;
            attackers |= (attacks::bishopAttacks(dst, allPieces) & allPieces & diagPieces);

            value = seePieceValue(PieceType::BISHOP) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard rooks = (stmAttackers & getPieces(PieceType::ROOK)))
        {
            Bitboard rook = rooks.lsbBB();
            allPieces ^= rook;
            attackers ^= rook;
            attackers |= (attacks::rookAttacks(dst, allPieces) & allPieces & straightPieces);

            value = seePieceValue(PieceType::ROOK) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard queens = (stmAttackers & getPieces(PieceType::QUEEN)))
        {
            Bitboard queen = queens.lsbBB();
            allPieces ^= queen;
            attackers ^= queen;
            attackers |=
                (attacks::bishopAttacks(dst, allPieces) & allPieces & diagPieces) |
                (attacks::rookAttacks(dst, allPieces) & allPieces & straightPieces);

            value = seePieceValue(PieceType::QUEEN) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (stmAttackers & getPieces(PieceType::KING))
        {
            if (attackers & getColor(~sideToMove))
            {
                return !us;
            }
            else
            {
                return us;
            }
        }

        us = !us;
    }

    return us;
}

bool Board::isLegal(Move move) const
{
    uint32_t kingIdx = getPieces(m_SideToMove, PieceType::KING).lsb();
    int from = move.srcPos();
    int to = move.dstPos();

    if (move.type() == MoveType::ENPASSANT)
    {
        int captureSq = to + (m_SideToMove == Color::WHITE ? -8 : 8);
        Bitboard piecesAfter = Bitboard::fromSquare(to) | (getAllPieces() ^ Bitboard::fromSquare(from) ^ Bitboard::fromSquare(captureSq));
        return !(attacks::rookAttacks(kingIdx, piecesAfter) & (getPieces(~m_SideToMove, PieceType::ROOK) | getPieces(~m_SideToMove, PieceType::QUEEN))) &&
            !(attacks::bishopAttacks(kingIdx, piecesAfter) & (getPieces(~m_SideToMove, PieceType::BISHOP) | getPieces(~m_SideToMove, PieceType::QUEEN)));
    }

    if (getPieceType(currState().squares[from]) == PieceType::KING)
    {
        if (move.type() == MoveType::CASTLE && squareAttacked(~m_SideToMove, (move.srcPos() + move.dstPos()) / 2))
            return false;
        return !squareAttacked(~m_SideToMove, move.dstPos(), getAllPieces() ^ Bitboard::fromSquare(from));
    }

    // pinned pieces
    return
        !(checkBlockers(m_SideToMove) & Bitboard::fromSquare(move.srcPos())) ||
        attacks::aligned(kingIdx, move.srcPos(), move.dstPos());
}

ZKey Board::keyAfter(Move move) const
{
    ZKey keyAfter = currState().zkey;
    int epSquare = currState().epSquare;

    keyAfter.flipSideToMove();

    if (epSquare != -1)
        keyAfter.updateEP(epSquare & 7);

    switch (move.type())
    {
        case MoveType::NONE:
        {
            Piece srcPiece = currState().squares[move.srcPos()];
            Piece dstPiece = currState().squares[move.dstPos()];

            if (dstPiece != Piece::NONE)
                keyAfter.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());

            keyAfter.movePiece(getPieceType(srcPiece), getPieceColor(srcPiece), move.srcPos(), move.dstPos());

            if (getPieceType(srcPiece) == PieceType::PAWN && std::abs(move.srcPos() - move.dstPos()) == 16)
                epSquare = (move.srcPos() + move.dstPos()) / 2;
            break;
        }
        case MoveType::PROMOTION:
        {
            Piece dstPiece = currState().squares[move.dstPos()];

            if (dstPiece != Piece::NONE)
                keyAfter.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.dstPos());

            keyAfter.removePiece(PieceType::PAWN, m_SideToMove, move.srcPos());
            keyAfter.addPiece(promoPiece(move.promotion()), m_SideToMove, move.dstPos());
            break;
        }
        case MoveType::CASTLE:
        {
            if (move.srcPos() > move.dstPos())
            {
                // queen side
                keyAfter.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
                keyAfter.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() - 2, move.srcPos() - 1);
            }
            else
            {
                // king side
                keyAfter.movePiece(PieceType::KING, m_SideToMove, move.srcPos(), move.dstPos());
                keyAfter.movePiece(PieceType::ROOK, m_SideToMove, move.dstPos() + 1, move.srcPos() + 1);
            }
            break;
        }
        case MoveType::ENPASSANT:
        {
            int offset = m_SideToMove == Color::WHITE ? -8 : 8;
            keyAfter.removePiece(PieceType::PAWN, ~m_SideToMove, move.dstPos() + offset);
            keyAfter.movePiece(PieceType::PAWN, m_SideToMove, move.srcPos(), move.dstPos());
            break;
        }
    }

    keyAfter.updateCastlingRights(currState().castlingRights);

    int newCastlingRights =
        currState().castlingRights &
        attacks::castleRightsMask(move.srcPos()) &
        attacks::castleRightsMask(move.dstPos());

    keyAfter.updateCastlingRights(newCastlingRights);



    if (epSquare == currState().epSquare)
        epSquare = -1;

    if (epSquare != -1)
        keyAfter.updateEP(epSquare & 7);

    return keyAfter;
}

void Board::updateCheckInfo()
{
    uint32_t whiteKingIdx = getPieces(Color::WHITE, PieceType::KING).lsb();
    uint32_t blackKingIdx = getPieces(Color::BLACK, PieceType::KING).lsb();
    uint32_t kingIdx = m_SideToMove == Color::WHITE ? whiteKingIdx : blackKingIdx;

    currState().checkInfo.checkers = attackersTo(~m_SideToMove, kingIdx);
    currState().checkInfo.blockers[static_cast<int>(Color::WHITE)] =
        pinnersBlockers(whiteKingIdx, getColor(Color::BLACK), currState().checkInfo.pinners[static_cast<int>(Color::WHITE)]);
    currState().checkInfo.blockers[static_cast<int>(Color::BLACK)] =
        pinnersBlockers(blackKingIdx, getColor(Color::WHITE), currState().checkInfo.pinners[static_cast<int>(Color::BLACK)]);
}

void Board::calcRepetitions()
{
    int reversible = std::min(currState().halfMoveClock, currState().pliesFromNull);
    for (int i = 2; i <= reversible; i += 2) {
        BoardState& state = m_States[m_States.size() - 1 - i];
        if (state.zkey == currState().zkey)
        {
            currState().repetitions = state.repetitions + 1;
            currState().lastRepetition = i;
            return;
        }
    }
    currState().repetitions = 0;
    currState().lastRepetition = 0;
}

void Board::addPiece(int pos, Color color, PieceType piece)
{
    currState().squares[pos] = makePiece(piece, color);

    Bitboard posBB = Bitboard::fromSquare(pos);
    currState().pieces[static_cast<int>(piece)] |= posBB;
    currState().colors[static_cast<int>(color)] |= posBB;

    currState().evalState.addPiece(color, piece, pos);
}

void Board::addPiece(int pos, Piece piece)
{
    currState().squares[pos] = piece;
    Bitboard posBB = Bitboard::fromSquare(pos);
    currState().pieces[static_cast<int>(getPieceType(piece))] |= posBB;
    currState().colors[static_cast<int>(getPieceColor(piece))] |= posBB;

    currState().evalState.addPiece(getPieceColor(piece), getPieceType(piece), pos);
}

void Board::removePiece(int pos)
{
    Bitboard posBB = Bitboard::fromSquare(pos);
    PieceType pieceType = getPieceType(currState().squares[pos]);
    Color color = getPieceColor(currState().squares[pos]);
    currState().squares[pos] = Piece::NONE;
    currState().pieces[static_cast<int>(pieceType)] ^= posBB;
    currState().colors[static_cast<int>(color)] ^= posBB;

    currState().evalState.removePiece(color, pieceType, pos);
}

void Board::movePiece(int src, int dst)
{
    Bitboard srcBB = Bitboard::fromSquare(src);
    Bitboard dstBB = Bitboard::fromSquare(dst);
    Bitboard moveBB = srcBB | dstBB;
    PieceType pieceType = getPieceType(currState().squares[src]);
    Color color = getPieceColor(currState().squares[src]);


    currState().squares[dst] = currState().squares[src];
    currState().squares[src] = Piece::NONE;
    currState().pieces[static_cast<int>(pieceType)] ^= moveBB;
    currState().colors[static_cast<int>(color)] ^= moveBB;

    currState().evalState.movePiece(color, pieceType, src, dst);
}
