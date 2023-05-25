#include "../defs.h"

#include <unordered_map>
#include <vector>
#include <fstream>

namespace comm
{

struct PGNHeader
{
	std::unordered_map<std::string, std::string> tags;
};

struct PGNEntry
{
	Move move;
	std::string comment;
};

struct PGNGame
{
	PGNHeader header;
	std::vector<PGNEntry> entries;
};

class PGNFile
{
public:
	PGNFile(const char* filename);
	~PGNFile();

	PGNGame parseGame();
	bool hasGame() const;
private:
	std::ifstream m_File;
	std::string m_Str;
	const char* m_Curr;
};

}