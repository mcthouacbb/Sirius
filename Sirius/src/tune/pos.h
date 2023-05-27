#pragma once

#include <vector>
#include <string>

namespace tune
{

struct Pos
{
	const char* epd;
	int epdLen;
	double result;
};

std::vector<Pos> parseEpdFile(const std::string& str);


}