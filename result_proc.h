#ifndef RESULT_PROC_H
#define RESULT_PROC_H

#include <string>
#include <list>
#include <mutex>

struct HitItem
{
  size_t Line;
  size_t Pos;
  std::string Data;
};

struct ChunkResult
{
  uint32_t ChunkIdx = 0;       // position of beginning of chunk
  size_t EndPnt_Line = 0;   // values need for calculating absolute lines and valid positions
  size_t EndPnt_Pos = 0;
  std::list<HitItem> HitList;
};

class ResultProcessor
{
public:
  void AddChunkResults(std::list<ChunkResult>&& chunkResults);
  void PostProcessing();
  std::string PrintResult() const;
private:
  std::mutex DataMutex;
  std::list<ChunkResult> TotalList;
  std::list<HitItem> FinalResult;
};


#endif // RESULT_PROC_H
