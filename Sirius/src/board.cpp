#include "board.h"
#include "attacks.h"
#include "eval/eval_state.h"
#include "movegen.h"
#include "cuckoo.h"
#include "util/string_split.h"

#include <cstring>
#include <charconv>

Board::Board()
{
    setToFen(defaultFen);
}

void Board::setToFen(const std::string_view& fen, bool frc)
{
    auto parts = splitBySpaces(fen);
    const auto& pieces = parts[0];
    const auto& stm = parts[1];
    const auto& castlingRights = parts[2];
    const auto& epSq = parts[3];

    m_States.clear();
    m_States.push_back(BoardState{});
    m_CastlingData = CastlingData();
    m_FRC = frc;

    currState().squares.fill(Piece::NONE);

    int i = 0;
    int sq = 56;
    for (;; i++)
    {
        switch (pieces[i])
        {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                sq += pieces[i] - '0';
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
    m_SideToMove = stm[0] == 'w' ? Color::WHITE : Color::BLACK;
    if (m_SideToMove == Color::BLACK)
    {
        currState().zkey.flipSideToMove();
    }

    currState().castlingRights = CastlingRights::NONE;
    m_CastlingData.setKingSquares(kingSq(Color::WHITE), kingSq(Color::BLACK));
    for (char c : castlingRights)
    {
        if (c == '-')
            break;
        Color color = std::isupper(c) ? Color::WHITE : Color::BLACK;
        c = std::tolower(c);
        if (c == 'k')
        {
            Square rookSq(kingSq(color).rank(), FILE_H);
            while (pieceAt(rookSq) != makePiece(PieceType::ROOK, color))
            {
                m_FRC = true;
                rookSq--;
            }
            m_CastlingData.setRookSquare(color, CastleSide::KING_SIDE, rookSq);
            if (color == Color::WHITE)
                currState().castlingRights |= CastlingRights::WHITE_KING_SIDE;
            else
                currState().castlingRights |= CastlingRights::BLACK_KING_SIDE;
        }
        else if (c == 'q')
        {
            Square rookSq(kingSq(color).rank(), FILE_A);
            while (pieceAt(rookSq) != makePiece(PieceType::ROOK, color))
            {
                m_FRC = true;
                rookSq++;
            }
            m_CastlingData.setRookSquare(color, CastleSide::QUEEN_SIDE, rookSq);
            if (color == Color::WHITE)
                currState().castlingRights |= CastlingRights::WHITE_QUEEN_SIDE;
            else
                currState().castlingRights |= CastlingRights::BLACK_QUEEN_SIDE;
        }
        else if (c >= 'a' && c <= 'h')
        {
            m_FRC = true;
            int file = c - 'a';
            CastleSide side = file > kingSq(color).file() ? CastleSide::KING_SIDE : CastleSide::QUEEN_SIDE;
            m_CastlingData.setRookSquare(color, side, Square(kingSq(color).rank(), file));
            if (side == CastleSide::KING_SIDE)
            {
                if (color == Color::WHITE)
                    currState().castlingRights |= CastlingRights::WHITE_KING_SIDE;
                else
                    currState().castlingRights |= CastlingRights::BLACK_KING_SIDE;
            }
            else
            {
                if (color == Color::WHITE)
                    currState().castlingRights |= CastlingRights::WHITE_QUEEN_SIDE;
                else
                    currState().castlingRights |= CastlingRights::BLACK_QUEEN_SIDE;
            }
        }
    }

    currState().zkey.updateCastlingRights(currState().castlingRights);

    if (epSq[0] != '-')
    {
        currState().epSquare = epSq[0] - 'a';
        currState().epSquare |= (epSq[1] - '1') << 3;
        currState().zkey.updateEP(currState().epSquare & 7);
    }
    else
    {
        currState().epSquare = -1;
    }

    if (parts.size() >= 6)
    {
        std::from_chars(&parts[4][0], &parts[4][0] + parts[4].size(), currState().halfMoveClock);
        std::from_chars(&parts[5][0], &parts[5][0] + parts[5].size(), m_GamePly);
        m_GamePly = 2 * m_GamePly - 1 - (m_SideToMove == Color::WHITE);
    }
    else
    {
        currState().halfMoveClock = 0;
        m_GamePly = 0;
    }

    m_CastlingData.initMasks();
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
            fen += !isFRC() ? 'K' : 'A' + castlingRookSq(Color::WHITE, CastleSide::KING_SIDE).file();
        if (currState().castlingRights.has(CastlingRights::WHITE_QUEEN_SIDE))
            fen += !isFRC() ? 'Q' : 'A' + castlingRookSq(Color::WHITE, CastleSide::QUEEN_SIDE).file();
        if (currState().castlingRights.has(CastlingRights::BLACK_KING_SIDE))
            fen += !isFRC() ? 'k' : 'a' + castlingRookSq(Color::BLACK, CastleSide::KING_SIDE).file();
        if (currState().castlingRights.has(CastlingRights::BLACK_QUEEN_SIDE))
            fen += !isFRC() ? 'q' : 'a' + castlingRookSq(Color::BLACK, CastleSide::QUEEN_SIDE).file();
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
    fen += std::to_string(m_GamePly / 2 + 1);

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
                removePiece(move.fromSq(), updates);
                removePiece(move.toSq(), updates);
                addPiece(castleKingDst(m_SideToMove, CastleSide::QUEEN_SIDE), makePiece(PieceType::KING, m_SideToMove), updates);
                addPiece(castleRookDst(m_SideToMove, CastleSide::QUEEN_SIDE), makePiece(PieceType::ROOK, m_SideToMove), updates);
            }
            else
            {
                // king side
                removePiece(move.fromSq(), updates);
                removePiece(move.toSq(), updates);
                addPiece(castleKingDst(m_SideToMove, CastleSide::KING_SIDE), makePiece(PieceType::KING, m_SideToMove), updates);
                addPiece(castleRookDst(m_SideToMove, CastleSide::KING_SIDE), makePiece(PieceType::ROOK, m_SideToMove), updates);
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

    currState().castlingRights &= m_CastlingData.castleRightsMask(move.fromSq());
    currState().castlingRights &= m_CastlingData.castleRightsMask(move.toSq());

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


bool Board::is50MoveDraw() const
{
    if (halfMoveClock() < 100)
        return false;
    if (checkers().empty())
        return true;
    
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(*this, moves);
    if (moves.size() == 0)
        return false;
    return true;
}

bool Board::isInsufMaterialDraw() const
{
    Bitboard nonMinorPcs =
        pieces(PieceType::QUEEN) |
        pieces(PieceType::ROOK) |
        pieces(PieceType::PAWN);

    if (nonMinorPcs.any())
        return false;

    switch (allPieces().popcount())
    {
        case 2:
            return true;
        case 3:
            return true;
        case 4:
        {
            Bitboard bishops = pieces(PieceType::BISHOP);
            if (bishops.popcount() == 2 && ((bishops & LIGHT_SQUARES_BB).popcount() == 2 || (bishops & LIGHT_SQUARES_BB).empty()))
                return true;
            return false;
        }
        default:
            return false;
    }
}

// see comment in cuckoo.cpp
bool Board::hasUpcomingRepetition(int searchPly) const
{
    const auto S = [this](int d)
    {
        return m_States[m_States.size() - 1 - d].zkey.value;
    };

    int reversible = std::min(currState().halfMoveClock, currState().pliesFromNull);
    if (reversible < 3)
        return false;

    uint64_t currKey = S(0);
    uint64_t other = ~(currKey ^ S(1));
    for (int i = 3; i <= reversible; i += 2)
    {
        uint64_t prevKey = S(i);
        other ^= ~(prevKey ^ S(i - 1));
        if (other != 0)
            continue;

        uint64_t keyDiff = currKey ^ prevKey;

        uint32_t slot = cuckoo::H1(keyDiff);
        if (keyDiff != cuckoo::keyDiffs[slot])
            slot = cuckoo::H2(keyDiff);

        if (keyDiff != cuckoo::keyDiffs[slot])
            continue;

        Move move = cuckoo::moves[slot];
        // move is obviously illegal if it goes through a piece
        if ((allPieces() & attacks::inBetweenSquares(move.fromSq(), move.toSq())).any())
            continue;

        if (searchPly > i)
            return true;

        if (m_States[m_States.size() - 1 - i].repetitions > 0)
            return true;
    }

    return false;
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

bool Board::castlingBlocked(Color color, CastleSide side) const
{
    return (m_CastlingData.blockSquares(color, side) & allPieces()).any();
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

    Bitboard whitePinned = checkBlockers(Color::WHITE) & pieces(Color::WHITE);
    Bitboard blackPinned = checkBlockers(Color::BLACK) & pieces(Color::BLACK);

    Bitboard whiteKingRay = attacks::alignedSquares(dst, kingSq(Color::WHITE));
    Bitboard blackKingRay = attacks::alignedSquares(dst, kingSq(Color::BLACK));

    Bitboard whitePinnedAligned = whiteKingRay & whitePinned;
    Bitboard blackPinnedAligned = blackKingRay & blackPinned;

    Bitboard pinned = whitePinned | blackPinned;
    Bitboard pinnedAligned = whitePinnedAligned | blackPinnedAligned;

    while (true)
    {
        sideToMove = ~sideToMove;
        Bitboard stmAttackers = attackers & pieces(sideToMove);
        if ((pinners(~sideToMove) & allPieces).any())
            stmAttackers &= ~pinned | pinnedAligned;

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
        if (move.type() == MoveType::CASTLE)
        {
            CastleSide side = to > from ? CastleSide::KING_SIDE : CastleSide::QUEEN_SIDE;
            Square rookTo = castleRookDst(m_SideToMove, side);
            Bitboard occ = allPieces() ^ Bitboard::fromSquare(from) ^ Bitboard::fromSquare(rookTo) ^ Bitboard::fromSquare(to);

            Square kingTo = castleKingDst(m_SideToMove, side);
            if (from != kingTo)
            {
                int step = kingTo > from ? 1 : -1;
                for (Square sq = from + step; sq != kingTo; sq += step)
                {
                    if (squareAttacked(~m_SideToMove, sq, occ))
                        return false;
                }
            }
            return !squareAttacked(~m_SideToMove, kingTo, occ);
        }
        return !squareAttacked(~m_SideToMove, move.toSq(), allPieces() ^ Bitboard::fromSquare(from));
    }

    // pinned pieces
    return
        !checkBlockers(m_SideToMove).has(move.fromSq()) ||
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
    PieceType srcPieceType = getPieceType(srcPiece);
    PieceType dstPieceType = getPieceType(dstPiece);

    if (move.type() == MoveType::CASTLE)
    {
        if (srcPieceType != PieceType::KING || checkers().any())
            return false;

        Bitboard firstRank = m_SideToMove == Color::WHITE ? Bitboard::nthRank<Color::WHITE, 0>() : Bitboard::nthRank<Color::BLACK, 0>();
        if (!firstRank.has(move.fromSq()) || !firstRank.has(move.toSq()))
            return false;

        if (move.fromSq() > move.toSq())
        {
            // queen side
            if (move.toSq() != castlingRookSq(m_SideToMove, CastleSide::QUEEN_SIDE))
                return false;
            if ((castlingRights().value() & (2 << 2 * static_cast<int>(m_SideToMove))) == 0)
                return false;
            return !castlingBlocked(m_SideToMove, CastleSide::QUEEN_SIDE);
        }
        else
        {
            // king side
            if (move.toSq() != castlingRookSq(m_SideToMove, CastleSide::KING_SIDE))
                return false;
            if ((castlingRights().value() & (1 << 2 * static_cast<int>(m_SideToMove))) == 0)
                return false;
            return !castlingBlocked(m_SideToMove, CastleSide::KING_SIDE);
        }
    }

    if (dstPiece != Piece::NONE && (getPieceColor(dstPiece) == m_SideToMove || getPieceType(dstPiece) == PieceType::KING))
        return false;

    if (srcPieceType != PieceType::KING && checkers().multiple())
        return false;

    if (move.type() == MoveType::CASTLE)
        return false;

    Bitboard moveMask = checkers().any() && srcPieceType != PieceType::KING ? attacks::moveMask(kingSq(m_SideToMove), checkers().lsb()) : Bitboard(~0ull);

    if (move.type() != MoveType::ENPASSANT && !moveMask.has(move.toSq()))
        return false;

    if (srcPieceType == PieceType::PAWN)
    {
        int pushOffset = m_SideToMove == Color::WHITE ? attacks::pawnPushOffset<Color::WHITE>() : attacks::pawnPushOffset<Color::BLACK>();
        if (move.type() == MoveType::ENPASSANT)
        {
            if (move.toSq().value() != epSquare())
                return false;
            if (!attacks::pawnAttacks(m_SideToMove, move.fromSq()).has(move.toSq()))
                return false;
            return checkers().empty() || moveMask.has(Square(epSquare() - pushOffset));
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
                    return seventhRank.has(move.fromSq());
                }
                else
                {
                    return move.fromSq().value() >= 8 && move.fromSq().value() < 56 && !seventhRank.has(move.fromSq());
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
                return secondRank.has(move.fromSq());
            }
            else
                return false;
        }
        else
        {
            // must be a capture
            if (!attacks::pawnAttacks(m_SideToMove, move.fromSq()).has(move.toSq()))
                return false;

            Bitboard seventhRank = m_SideToMove == Color::WHITE ? Bitboard::nthRank<Color::WHITE, 6>() : Bitboard::nthRank<Color::BLACK, 6>();

            if (move.type() == MoveType::PROMOTION)
            {
                return seventhRank.has(move.fromSq());
            }
            else
            {
                return move.fromSq().value() >= 8 && move.fromSq().value() < 56 && !seventhRank.has(move.fromSq());
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

    return pieceAttacks.has(move.toSq());
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
                keyAfter.removePiece(PieceType::KING, m_SideToMove, move.fromSq());
                keyAfter.removePiece(PieceType::ROOK, m_SideToMove, move.toSq());
                keyAfter.addPiece(PieceType::KING, m_SideToMove, castleKingDst(m_SideToMove, CastleSide::QUEEN_SIDE));
                keyAfter.addPiece(PieceType::ROOK, m_SideToMove, castleRookDst(m_SideToMove, CastleSide::QUEEN_SIDE));
            }
            else
            {
                // king side
                keyAfter.removePiece(PieceType::KING, m_SideToMove, move.fromSq());
                keyAfter.removePiece(PieceType::ROOK, m_SideToMove, move.toSq());
                keyAfter.addPiece(PieceType::KING, m_SideToMove, castleKingDst(m_SideToMove, CastleSide::KING_SIDE));
                keyAfter.addPiece(PieceType::ROOK, m_SideToMove, castleRookDst(m_SideToMove, CastleSide::KING_SIDE));
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
        m_CastlingData.castleRightsMask(move.fromSq()) &
        m_CastlingData.castleRightsMask(move.toSq());

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
    for (int i = 4; i <= reversible; i += 2)
    {
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
