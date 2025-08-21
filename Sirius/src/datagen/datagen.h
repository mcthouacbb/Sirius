#include <cstdint>
#include <mutex>
#include <string>

void runDatagen(uint32_t threadID, std::string filename, std::mutex& coutLock);
