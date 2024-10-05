#include "board.h"
#include "attacks.h"
#include "eval/eval_state.h"

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

    currState().zkey.value = 0;
    currState().pawnKey.value = 0;
    currState().nonPawnKey[static_cast<int>(Color::WHITE)].value = 0;
    currState().nonPawnKey[static_cast<int>(Color::BLACK)].value = 0;
    currState().minorPieceKey.value = 0;

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
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::KING);
                break;
            case 'q':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::QUEEN);
                break;
            case 'r':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::ROOK);
                break;
            case 'b':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::BISHOP);
                break;
            case 'n':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::KNIGHT);
                break;
            case 'p':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::PAWN);
                break;
            case 'K':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::KING);
                break;
            case 'Q':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::QUEEN);
                break;
            case 'R':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::ROOK);
                break;
            case 'B':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::BISHOP);
                break;
            case 'N':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::KNIGHT);
                break;
            case 'P':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::PAWN);
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
    currState().castlingRights = CastlingRights::NONE;
    if (fen[i] == '-')
    {
        i++;
    }
    else
    {
        if (fen[i] == 'K')
        {
            i++;
            currState().castlingRights |= CastlingRights::WHITE_KING_SIDE;
        }

        if (fen[i] == 'Q')
        {
            i++;
            currState().castlingRights |= CastlingRights::WHITE_QUEEN_SIDE;
        }

        if (fen[i] == 'k')
        {
            i++;
            currState().castlingRights |= CastlingRights::BLACK_KING_SIDE;
        }

        if (fen[i] == 'q')
        {
            i++;
            currState().castlingRights |= CastlingRights::BLACK_QUEEN_SIDE;
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
    calcThreats();
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

    currState().zkey.value = 0;
    currState().pawnKey.value = 0;
    currState().nonPawnKey[static_cast<int>(Color::WHITE)].value = 0;
    currState().nonPawnKey[static_cast<int>(Color::BLACK)].value = 0;
    currState().minorPieceKey.value = 0;

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
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::KING);
                break;
            case 'q':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::QUEEN);
                break;
            case 'r':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::ROOK);
                break;
            case 'b':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::BISHOP);
                break;
            case 'n':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::KNIGHT);
                break;
            case 'p':
                currState().addPiece(Square(sq++), Color::BLACK, PieceType::PAWN);
                break;
            case 'K':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::KING);
                break;
            case 'Q':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::QUEEN);
                break;
            case 'R':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::ROOK);
                break;
            case 'B':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::BISHOP);
                break;
            case 'N':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::KNIGHT);
                break;
            case 'P':
                currState().addPiece(Square(sq++), Color::WHITE, PieceType::PAWN);
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
    currState().castlingRights = CastlingRights::NONE;
    if (epd[i] == '-')
    {
        i++;
    }
    else
    {
        if (epd[i] == 'K')
        {
            i++;
            currState().castlingRights |= CastlingRights::WHITE_KING_SIDE;
        }

        if (epd[i] == 'Q')
        {
            i++;
            currState().castlingRights |= CastlingRights::WHITE_QUEEN_SIDE;
        }

        if (epd[i] == 'k')
        {
            i++;
            currState().castlingRights |= CastlingRights::BLACK_KING_SIDE;
        }

        if (epd[i] == 'q')
        {
            i++;
            currState().castlingRights |= CastlingRights::BLACK_QUEEN_SIDE;
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
    calcThreats();
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

    if (currState().castlingRights.value() == 0)
        fen += '-';
    else
    {
        if (currState().castlingRights.has(CastlingRights::WHITE_KING_SIDE))
            fen += 'K';
        if (currState().castlingRights.has(CastlingRights::WHITE_QUEEN_SIDE))
            fen += 'Q';
        if (currState().castlingRights.has(CastlingRights::BLACK_KING_SIDE))
            fen += 'k';
        if (currState().castlingRights.has(CastlingRights::BLACK_QUEEN_SIDE))
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

    if (currState().castlingRights.value() == 0)
        epd += '-';
    else
    {
        if (currState().castlingRights.has(CastlingRights::WHITE_KING_SIDE))
            epd += 'K';
        if (currState().castlingRights.has(CastlingRights::WHITE_QUEEN_SIDE))
            epd += 'Q';
        if (currState().castlingRights.has(CastlingRights::BLACK_KING_SIDE))
            epd += 'k';
        if (currState().castlingRights.has(CastlingRights::BLACK_QUEEN_SIDE))
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

template<bool updateEval>
void Board::makeMove(Move move, eval::EvalState* evalState)
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

    eval::EvalUpdates updates;

    switch (move.type())
    {
        case MoveType::NONE:
        {
            Piece srcPiece = pieceAt(move.fromSq());

            Piece dstPiece = pieceAt(move.toSq());

            if (dstPiece != Piece::NONE)
            {
                currState().halfMoveClock = 0;
                removePiece(move.toSq(), updates);
            }

            movePiece(move.fromSq(), move.toSq(), updates);

            if (getPieceType(srcPiece) == PieceType::PAWN)
            {
                currState().halfMoveClock = 0;
                if (std::abs(move.fromSq() - move.toSq()) == 16)
                    currState().epSquare = (move.fromSq().value() + move.toSq().value()) / 2;
            }
            break;
        }
        case MoveType::PROMOTION:
        {
            currState().halfMoveClock = 0;

            Piece dstPiece = pieceAt(move.toSq());

            if (dstPiece != Piece::NONE)
                removePiece(move.toSq(), updates);
            removePiece(move.fromSq(), updates);
            addPiece(move.toSq(), m_SideToMove, promoPiece(move.promotion()), updates);
            break;
        }
        case MoveType::CASTLE:
        {
            if (move.fromSq() > move.toSq())
            {
                // queen side
                movePiece(move.fromSq(), move.toSq(), updates);
                movePiece(move.toSq() - 2, move.fromSq() - 1, updates);
            }
            else
            {
                // king side
                movePiece(move.fromSq(), move.toSq(), updates);
                movePiece(move.toSq() + 1, move.fromSq() + 1, updates);
            }
            break;
        }
        case MoveType::ENPASSANT:
        {
            currState().halfMoveClock = 0;

            int offset = m_SideToMove == Color::WHITE ? -8 : 8;

            removePiece(move.toSq() + offset, updates);
            movePiece(move.fromSq(), move.toSq(), updates);
            break;
        }
    }

    currState().zkey.updateCastlingRights(currState().castlingRights);

    currState().castlingRights &= attacks::castleRightsMask(move.fromSq());
    currState().castlingRights &= attacks::castleRightsMask(move.toSq());

    currState().zkey.updateCastlingRights(currState().castlingRights);



    if (currState().epSquare == prev.epSquare)
        currState().epSquare = -1;

    if (currState().epSquare != -1)
        currState().zkey.updateEP(currState().epSquare & 7);

    m_SideToMove = ~m_SideToMove;

    updateCheckInfo();
    calcThreats();
    calcRepetitions();

    if constexpr (updateEval)
        evalState->push(*this, updates);
}

template<bool updateEval>
void Board::unmakeMove(eval::EvalState* evalState)
{
    m_States.pop_back();
    m_GamePly--;

    m_SideToMove = ~m_SideToMove;

    if constexpr (updateEval)
        evalState->pop();
}

template void Board::makeMove<true>(Move move, eval::EvalState* evalState);
template void Board::makeMove<false>(Move move, eval::EvalState* evalState);

template void Board::unmakeMove<true>(eval::EvalState* evalState);
template void Board::unmakeMove<false>(eval::EvalState* evalState);

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
    calcThreats();
}

void Board::unmakeNullMove()
{
    m_States.pop_back();

    m_GamePly--;

    m_SideToMove = ~m_SideToMove;
}

bool Board::squareAttacked(Color color, Square square, Bitboard blockers) const
{
    return attackersTo(color, square, blockers).any();
}

Bitboard Board::attackersTo(Color color, Square square, Bitboard blockers) const
{
    Bitboard queens = pieces(PieceType::QUEEN);
    Bitboard pawns = (pieces(color, PieceType::PAWN) & attacks::pawnAttacks(~color, square));
    Bitboard nonPawns =
        (pieces(PieceType::KNIGHT) & attacks::knightAttacks(square)) |
        (pieces(PieceType::KING) & attacks::kingAttacks(square)) |
        ((pieces(PieceType::BISHOP) | queens) & attacks::bishopAttacks(square, blockers)) |
        ((pieces(PieceType::ROOK) | queens) & attacks::rookAttacks(square, blockers));
    return pawns | (nonPawns & pieces(color));
}

Bitboard Board::attackersTo(Square square, Bitboard blockers) const
{
    Bitboard queens = pieces(PieceType::QUEEN);
    Bitboard pawns =
        (pieces(Color::WHITE, PieceType::PAWN) & attacks::pawnAttacks(Color::BLACK, square)) |
        (pieces(Color::BLACK, PieceType::PAWN) & attacks::pawnAttacks(Color::WHITE, square));

    Bitboard nonPawns =
        (pieces(PieceType::KNIGHT) & attacks::knightAttacks(square)) |
        (pieces(PieceType::KING) & attacks::kingAttacks(square)) |
        ((pieces(PieceType::BISHOP) | queens) & attacks::bishopAttacks(square, blockers)) |
        ((pieces(PieceType::ROOK) | queens) & attacks::rookAttacks(square, blockers));
    return pawns | nonPawns;
}

bool Board::isPassedPawn(Square square) const
{
    Piece pce = pieceAt(square);
    Bitboard mask = attacks::passedPawnMask(getPieceColor(pce), square);
    return (mask & pieces(~getPieceColor(pce), PieceType::PAWN)).empty();
}

bool Board::isIsolatedPawn(Square square) const
{
    Piece pce = pieceAt(square);
    Bitboard mask = attacks::isolatedPawnMask(square);
    return (mask & pieces(getPieceColor(pce), PieceType::PAWN)).empty();
}

Bitboard Board::pinnersBlockers(Square square, Bitboard attackers, Bitboard& pinners) const
{
    Bitboard queens = pieces(PieceType::QUEEN);
    attackers &=
        (attacks::rookAttacks(square, Bitboard(0)) & (pieces(PieceType::ROOK) | queens)) |
        (attacks::bishopAttacks(square, Bitboard(0)) & (pieces(PieceType::BISHOP) | queens));

    Bitboard blockers = Bitboard(0);

    Bitboard blockMask = allPieces() ^ attackers;

    Bitboard sameColor = pieces(getPieceColor(pieceAt(square)));

    while (attackers.any())
    {
        Square attacker = attackers.poplsb();

        Bitboard between = attacks::inBetweenSquares(square, attacker) & blockMask;

        if (between.one())
        {
            blockers |= between;
            if ((between & sameColor).any())
                pinners |= Bitboard::fromSquare(attacker);
        }
    }
    return blockers;
}

// mostly from stockfish
bool Board::see(Move move, int margin) const
{
    Square src = move.fromSq();
    Square dst = move.toSq();

    Bitboard allPieces = this->allPieces() ^ Bitboard::fromSquare(src);
    Bitboard attackers = attackersTo(dst, allPieces) ^ Bitboard::fromSquare(src);

    int value = 0;
    switch (move.type())
    {
        case MoveType::CASTLE:
            // rook and king cannot be attacked after castle
            return 0 >= margin;
        case MoveType::NONE:
        {
            PieceType type = getPieceType(pieceAt(dst));
            if (type == PieceType::NONE)
                value = -margin;
            else
                value = seePieceValue(type) - margin;
            if (value < 0)
                return false;
            value = seePieceValue(getPieceType(pieceAt(src))) - value;
            break;
        }
        case MoveType::PROMOTION:
        {
            PieceType promo = promoPiece(move.promotion());
            PieceType type = getPieceType(pieceAt(dst));
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

    Bitboard diagPieces = pieces(PieceType::BISHOP) | pieces(PieceType::QUEEN);
    Bitboard straightPieces = pieces(PieceType::ROOK) | pieces(PieceType::QUEEN);

    bool us = false;

    while (true)
    {
        sideToMove = ~sideToMove;
        Bitboard stmAttackers = attackers & pieces(sideToMove);
        if (stmAttackers.empty())
            return !us;

        if (Bitboard pawns = (stmAttackers & pieces(PieceType::PAWN)); pawns.any())
        {
            Bitboard pawn = pawns.lsbBB();
            allPieces ^= pawn;
            attackers ^= pawn;
            attackers |= (attacks::bishopAttacks(dst, allPieces) & allPieces & diagPieces);

            value = seePieceValue(PieceType::PAWN) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard knights = (stmAttackers & pieces(PieceType::KNIGHT)); knights.any())
        {
            Bitboard knight = knights.lsbBB();
            allPieces ^= knight;
            attackers ^= knight;

            value = seePieceValue(PieceType::KNIGHT) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard bishops = (stmAttackers & pieces(PieceType::BISHOP)); bishops.any())
        {
            Bitboard bishop = bishops.lsbBB();
            allPieces ^= bishop;
            attackers ^= bishop;
            attackers |= (attacks::bishopAttacks(dst, allPieces) & allPieces & diagPieces);

            value = seePieceValue(PieceType::BISHOP) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard rooks = (stmAttackers & pieces(PieceType::ROOK)); rooks.any())
        {
            Bitboard rook = rooks.lsbBB();
            allPieces ^= rook;
            attackers ^= rook;
            attackers |= (attacks::rookAttacks(dst, allPieces) & allPieces & straightPieces);

            value = seePieceValue(PieceType::ROOK) - value;
            if (value < static_cast<int>(us))
                return us;
        }
        else if (Bitboard queens = (stmAttackers & pieces(PieceType::QUEEN)); queens.any())
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
        else if ((stmAttackers & pieces(PieceType::KING)).any())
        {
            if ((attackers & pieces(~sideToMove)).any())
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
    Square from = move.fromSq();
    Square to = move.toSq();

    if (move.type() == MoveType::ENPASSANT)
    {
        Square captureSq = to + (m_SideToMove == Color::WHITE ? -8 : 8);
        Bitboard piecesAfter = Bitboard::fromSquare(to) | (allPieces() ^ Bitboard::fromSquare(from) ^ Bitboard::fromSquare(captureSq));
        return (attacks::rookAttacks(kingSq(m_SideToMove), piecesAfter) & (pieces(~m_SideToMove, PieceType::ROOK) | pieces(~m_SideToMove, PieceType::QUEEN))).empty() &&
            (attacks::bishopAttacks(kingSq(m_SideToMove), piecesAfter) & (pieces(~m_SideToMove, PieceType::BISHOP) | pieces(~m_SideToMove, PieceType::QUEEN))).empty();
    }

    if (getPieceType(pieceAt(from)) == PieceType::KING)
    {
        if (move.type() == MoveType::CASTLE && squareAttacked(~m_SideToMove, Square::average(move.fromSq(), move.toSq())))
            return false;
        return !squareAttacked(~m_SideToMove, move.toSq(), allPieces() ^ Bitboard::fromSquare(from));
    }

    // pinned pieces
    return
        (checkBlockers(m_SideToMove) & Bitboard::fromSquare(move.fromSq())).empty() ||
        attacks::aligned(kingSq(m_SideToMove), move.fromSq(), move.toSq());
}

// move generation handles double check and check evasions, so this function also handles them
bool Board::isPseudoLegal(Move move) const
{
    if (move.fromSq() == move.toSq())
        return false;

    Piece srcPiece = pieceAt(move.fromSq());
    if (srcPiece == Piece::NONE || getPieceColor(srcPiece) != m_SideToMove)
        return false;

    Piece dstPiece = pieceAt(move.toSq());
    if (dstPiece != Piece::NONE && (getPieceColor(dstPiece) == m_SideToMove || getPieceType(dstPiece) == PieceType::KING))
        return false;

    PieceType srcPieceType = getPieceType(srcPiece);
    PieceType dstPieceType = getPieceType(dstPiece);

    if (move.type() == MoveType::CASTLE)
    {
        if (srcPieceType != PieceType::KING || checkers().any())
            return false;

        Bitboard firstRank = m_SideToMove == Color::WHITE ? Bitboard::nthRank<Color::WHITE, 0>() : Bitboard::nthRank<Color::BLACK, 0>();
        if ((Bitboard::fromSquare(move.fromSq()) & firstRank).empty() || (Bitboard::fromSquare(move.toSq()) & firstRank).empty())
            return false;

        if (move.fromSq() > move.toSq())
        {
            if (move.toSq() != move.fromSq() - 2)
                return false;
            // queen side
            if ((castlingRights().value() & (2 << 2 * static_cast<int>(m_SideToMove))) == 0)
                return false;
            if (m_SideToMove == Color::WHITE)
                return (attacks::qscBlockSquares<Color::WHITE>() & allPieces()).empty();
            else
                return (attacks::qscBlockSquares<Color::BLACK>() & allPieces()).empty();
        }
        else
        {
            if (move.toSq() != move.fromSq() + 2)
                return false;
            // king side
            if ((castlingRights().value() & (1 << 2 * static_cast<int>(m_SideToMove))) == 0)
                return false;
            if (m_SideToMove == Color::WHITE)
                return (attacks::kscBlockSquares<Color::WHITE>() & allPieces()).empty();
            else
                return (attacks::kscBlockSquares<Color::BLACK>() & allPieces()).empty();
        }
    }

    if (srcPieceType != PieceType::KING && checkers().multiple())
        return false;

    if (move.type() == MoveType::CASTLE)
        return false;

    Bitboard moveMask = checkers().any() && srcPieceType != PieceType::KING ? attacks::moveMask(kingSq(m_SideToMove), checkers().lsb()) : Bitboard(~0ull);

    if (move.type() != MoveType::ENPASSANT && (Bitboard::fromSquare(move.toSq()) & moveMask).empty())
        return false;

    if (srcPieceType == PieceType::PAWN)
    {
        int pushOffset = m_SideToMove == Color::WHITE ? attacks::pawnPushOffset<Color::WHITE>() : attacks::pawnPushOffset<Color::BLACK>();
        if (move.type() == MoveType::ENPASSANT)
        {
            if (move.toSq().value() != epSquare())
                return false;
            if ((attacks::pawnAttacks(m_SideToMove, move.fromSq()) & Bitboard::fromSquare(move.toSq())).empty())
                return false;
            return checkers().empty() || (moveMask & Bitboard::fromSquare(Square(epSquare() - pushOffset))).any();
        }

        if (dstPieceType == PieceType::NONE)
        {
            // must be a push
            if (move.toSq() - move.fromSq() == pushOffset)
            {
                // single
                Bitboard seventhRank = m_SideToMove == Color::WHITE ? Bitboard::nthRank<Color::WHITE, 6>() : Bitboard::nthRank<Color::BLACK, 6>();

                if (move.type() == MoveType::PROMOTION)
                {
                    return (Bitboard::fromSquare(move.fromSq()) & seventhRank).any();
                }
                else
                {
                    return move.fromSq().value() >= 8 && move.fromSq().value() < 56 && (Bitboard::fromSquare(move.fromSq()) & seventhRank).empty();
                }
            }
            else if (move.toSq() - move.fromSq() == pushOffset * 2)
            {
                if (move.type() != MoveType::NONE)
                    return false;
                if (pieceAt(move.fromSq() + pushOffset) != Piece::NONE)
                    return false;
                // double
                Bitboard secondRank = m_SideToMove == Color::WHITE ? Bitboard::nthRank<Color::WHITE, 1>() : Bitboard::nthRank<Color::BLACK, 1>();
                return (Bitboard::fromSquare(move.fromSq()) & secondRank).any();
            }
            else
                return false;
        }
        else
        {
            // must be a capture
            if ((attacks::pawnAttacks(m_SideToMove, move.fromSq()) & Bitboard::fromSquare(move.toSq())).empty())
                return false;

            Bitboard seventhRank = m_SideToMove == Color::WHITE ? Bitboard::nthRank<Color::WHITE, 6>() : Bitboard::nthRank<Color::BLACK, 6>();

            if (move.type() == MoveType::PROMOTION)
            {
                return (Bitboard::fromSquare(move.fromSq()) & seventhRank).any();
            }
            else
            {
                return move.fromSq().value() >= 8 && move.fromSq().value() < 56 && (Bitboard::fromSquare(move.fromSq()) & seventhRank).empty();
            }
        }
    }

    if (move.type() != MoveType::NONE)
        return false;

    Bitboard pieceAttacks = {};
    switch (srcPieceType)
    {
        case PieceType::KNIGHT:
            pieceAttacks = attacks::knightAttacks(move.fromSq());
            break;
        case PieceType::BISHOP:
            pieceAttacks = attacks::bishopAttacks(move.fromSq(), allPieces());
            break;
        case PieceType::ROOK:
            pieceAttacks = attacks::rookAttacks(move.fromSq(), allPieces());
            break;
        case PieceType::QUEEN:
            pieceAttacks = attacks::queenAttacks(move.fromSq(), allPieces());
            break;
        case PieceType::KING:
            pieceAttacks = attacks::kingAttacks(move.fromSq());
            break;
    }

    return (pieceAttacks & Bitboard::fromSquare(move.toSq())).any();
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
            Piece srcPiece = pieceAt(move.fromSq());
            Piece dstPiece = pieceAt(move.toSq());

            if (dstPiece != Piece::NONE)
                keyAfter.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.toSq());

            keyAfter.movePiece(getPieceType(srcPiece), getPieceColor(srcPiece), move.fromSq(), move.toSq());

            if (getPieceType(srcPiece) == PieceType::PAWN && std::abs(move.fromSq() - move.toSq()) == 16)
                epSquare = Square::average(move.fromSq(), move.toSq()).value();
            break;
        }
        case MoveType::PROMOTION:
        {
            Piece dstPiece = pieceAt(move.toSq());

            if (dstPiece != Piece::NONE)
                keyAfter.removePiece(getPieceType(dstPiece), getPieceColor(dstPiece), move.toSq());

            keyAfter.removePiece(PieceType::PAWN, m_SideToMove, move.fromSq());
            keyAfter.addPiece(promoPiece(move.promotion()), m_SideToMove, move.toSq());
            break;
        }
        case MoveType::CASTLE:
        {
            if (move.fromSq() > move.toSq())
            {
                // queen side
                keyAfter.movePiece(PieceType::KING, m_SideToMove, move.fromSq(), move.toSq());
                keyAfter.movePiece(PieceType::ROOK, m_SideToMove, move.toSq() - 2, move.fromSq() - 1);
            }
            else
            {
                // king side
                keyAfter.movePiece(PieceType::KING, m_SideToMove, move.fromSq(), move.toSq());
                keyAfter.movePiece(PieceType::ROOK, m_SideToMove, move.toSq() + 1, move.fromSq() + 1);
            }
            break;
        }
        case MoveType::ENPASSANT:
        {
            int offset = m_SideToMove == Color::WHITE ? -8 : 8;
            keyAfter.removePiece(PieceType::PAWN, ~m_SideToMove, move.toSq() + offset);
            keyAfter.movePiece(PieceType::PAWN, m_SideToMove, move.fromSq(), move.toSq());
            break;
        }
    }

    keyAfter.updateCastlingRights(currState().castlingRights);

    CastlingRights newCastlingRights =
        currState().castlingRights &
        attacks::castleRightsMask(move.fromSq()) &
        attacks::castleRightsMask(move.toSq());

    keyAfter.updateCastlingRights(newCastlingRights);



    if (epSquare == currState().epSquare)
        epSquare = -1;

    if (epSquare != -1)
        keyAfter.updateEP(epSquare & 7);

    return keyAfter;
}

void Board::updateCheckInfo()
{
    Square whiteKingSq = kingSq(Color::WHITE);
    Square blackKingSq = kingSq(Color::BLACK);
    Square kingSq = m_SideToMove == Color::WHITE ? whiteKingSq : blackKingSq;

    currState().checkInfo.checkers = attackersTo(~m_SideToMove, kingSq);
    currState().checkInfo.blockers[static_cast<int>(Color::WHITE)] =
        pinnersBlockers(whiteKingSq, pieces(Color::BLACK), currState().checkInfo.pinners[static_cast<int>(Color::WHITE)]);
    currState().checkInfo.blockers[static_cast<int>(Color::BLACK)] =
        pinnersBlockers(blackKingSq, pieces(Color::WHITE), currState().checkInfo.pinners[static_cast<int>(Color::BLACK)]);
}

void Board::calcThreats()
{
    Color color = ~m_SideToMove;
    Bitboard threats = Bitboard(0);
    Bitboard occupied = allPieces();

    Bitboard queens = pieces(color, PieceType::QUEEN);
    Bitboard rooks = pieces(color, PieceType::ROOK);
    Bitboard bishops = pieces(color, PieceType::BISHOP);
    Bitboard knights = pieces(color, PieceType::KNIGHT);
    Bitboard pawns = pieces(color, PieceType::PAWN);

    rooks |= queens;
    while (rooks.any())
    {
        Square rook = rooks.poplsb();
        threats |= attacks::rookAttacks(rook, occupied);
    }

    bishops |= queens;
    while (bishops.any())
    {
        Square bishop = bishops.poplsb();
        threats |= attacks::bishopAttacks(bishop, occupied);
    }

    while (knights.any())
    {
        Square knight = knights.poplsb();
        threats |= attacks::knightAttacks(knight);
    }

    if (color == Color::WHITE)
        threats |= attacks::pawnAttacks<Color::WHITE>(pawns);
    else
        threats |= attacks::pawnAttacks<Color::BLACK>(pawns);

    threats |= attacks::kingAttacks(kingSq(color));

    currState().threats = threats;
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

void Board::addPiece(Square pos, Color color, PieceType pieceType, eval::EvalUpdates& updates)
{
    Piece piece = makePiece(pieceType, color);
    currState().addPiece(pos, piece);
    updates.pushAdd({piece, pos});
}

void Board::addPiece(Square pos, Piece piece, eval::EvalUpdates& updates)
{
    currState().addPiece(pos, piece);
    updates.pushAdd({piece, pos});
}

void Board::removePiece(Square pos, eval::EvalUpdates& updates)
{
    Piece piece = pieceAt(pos);
    currState().removePiece(pos);
    updates.pushRemove({piece, pos});
}

void Board::movePiece(Square src, Square dst, eval::EvalUpdates& updates)
{
    Piece piece = pieceAt(src);
    currState().movePiece(src, dst);
    updates.pushRemove({piece, src});
    updates.pushAdd({piece, dst});
}
