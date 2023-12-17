#include "zobrist.h"
#include "util/prng.h"

namespace zobrist
{

Keys keys;

void init()
{
    PRNG prng;
    prng.seed(8367428251681ull);

    for (int color = 0; color < 2; color++)
        for (int piece = 0; piece < 6; piece++)
            for (int square = 0; square < 64; square++)
                keys.pieceSquares[color][5 - piece][square] = prng.next64();

    for (int i = 0; i < 16; i++)
        keys.castlingRights[i] = prng.next64();

    for (int i = 0; i < 8; i++)
        keys.epFiles[i] = prng.next64();

    keys.blackToMove = prng.next64();
}

}
