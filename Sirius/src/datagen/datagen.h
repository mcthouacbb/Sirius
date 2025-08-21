#include <cstdint>
#include <mutex>
#include <string>

namespace datagen
{

void runDatagen(uint32_t threadID, std::string filename, std::mutex& coutLock);

}
