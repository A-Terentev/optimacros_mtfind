#include "result_proc.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>

void ResultProcessor::AddChunkResults(std::list<ChunkResult>&& chunkResults)
{
  std::lock_guard<std::mutex> lock(DataMutex);
  TotalList.splice(TotalList.end(), chunkResults);
}

static bool Cmp(const ChunkResult &a, const ChunkResult &b)
{
  return a.ChunkIdx < b.ChunkIdx;
}

void ResultProcessor::PostProcessing()
{
  TotalList.sort(Cmp);

  size_t absLine = 0;
  size_t endLinePos = 0;
  std::list<HitItem> finResult;
  auto trfrm = [&absLine, &endLinePos, &finResult](ChunkResult& chunkRes)
  {
    for (HitItem& hit : chunkRes.HitList)
    {
      hit.Pos += (hit.Line == 0 ? endLinePos : 0);
      hit.Line += absLine;
      finResult.push_back(std::move(hit));
    }
    absLine += chunkRes.EndPnt_Line;
    endLinePos = chunkRes.EndPnt_Pos;
  };
  std::for_each(TotalList.begin(), TotalList.end(), trfrm);

  FinalResult.swap(finResult);
}

std::string ResultProcessor::PrintResult() const
{
  auto makeLine = [](std::string a, const HitItem& hit)
  {
    return std::move(a) + std::to_string(hit.Line + 1) + ' ' + std::to_string(hit.Pos + 1) + ' ' + hit.Data + "\n";
  };
  return std::accumulate(FinalResult.begin(), FinalResult.end(), std::to_string(FinalResult.size()) + "\n", makeLine);
}
