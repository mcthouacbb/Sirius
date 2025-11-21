#include "stats.h"
#include <array>
#include <fstream>
#include <iostream>

#include "../defs.h"

namespace datagen
{

void computeStats(std::string fensFile)
{
    std::ifstream file(fensFile);
    std::string line;

    std::array<u32, 33> pieceCounts = {};
    std::array<u32, 25> phaseCounts = {};
    std::array<u32, 17> pawnCounts = {};
    std::array<std::array<u32, 17>, 25> pawnPhaseCounts = {};

    while (std::getline(file, line))
    {
        i32 phase = 0;
        i32 pawnCount = 0;
        i32 pieceCount = 0;
        for (char c : line)
        {
            if (c == ' ')
                break;
            if (std::isalpha(c))
                pieceCount++;
            switch (c)
            {
                case 'p':
                case 'P':
                    pawnCount++;
                    break;
                case 'n':
                case 'N':
                case 'b':
                case 'B':
                    phase++;
                    break;
                case 'r':
                case 'R':
                    phase += 2;
                    break;
                case 'q':
                case 'Q':
                    phase += 4;
                    break;
            }
        }
        pieceCounts[pieceCount]++;
        phaseCounts[phase]++;
        pawnCounts[pawnCount]++;
        pawnPhaseCounts[phase][pawnCount]++;
    }

    for (i32 i = 0; i <= 32; i++)
        std::cout << pieceCounts[i] << std::endl;

    for (i32 i = 0; i <= 24; i++)
        std::cout << phaseCounts[i] << std::endl;

    for (i32 i = 0; i <= 16; i++)
        std::cout << pawnCounts[i] << std::endl;

    for (i32 i = 0; i <= 24; i++)
        for (i32 j = 0; j <= 16; j++)
            std::cout << pawnPhaseCounts[i][j] << std::endl;
}

}
