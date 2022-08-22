#include "data_supp.h"
//#include "result_writer.h"
//#include <functional>
//#include <limits.h>
#include "result_proc.h"
#include <iostream>
#include <list>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <numeric>


#define NUM_OF_CALC_THREADS 4

struct TimeCounters
{
  std::chrono::duration<double> ReadTime{0};
  std::chrono::duration<double> SearchTime{0};
};

class ExecTimers
{
public:
  void AddExecuterTime(TimeCounters times)
  {
    std::lock_guard<std::mutex> lock(DataMutex);
    Times.push_back(times);
  }
  std::string GetInfo() const
  {
    std::string res;
    // calc time of reading data chunks and searching substr by executers
    TimeCounters sum = std::accumulate(Times.begin(), Times.end(), TimeCounters(),
                                       [](TimeCounters a, TimeCounters b) { return TimeCounters{a.ReadTime + b.ReadTime, a.SearchTime + b.SearchTime};});
    res = "Per thread average Read time: ";
    res += std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(sum.ReadTime).count() / Times.size());
    res += " us\n";
    res += "Per thread average Search time: ";
    res += std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(sum.SearchTime).count() / Times.size());
    res += " us\n";
    return res;
  }

private:
  std::list<TimeCounters> Times;
  std::mutex DataMutex;
};

static ExecTimers ExecutersTimers;

static ChunkResult ProcChunk(const std::vector<char>& src, const char* const mask, const uint8_t maskLen)
{
  ChunkResult res;
  std::uint32_t line = 0;    // index of line in chunk
  std::uint32_t linePos = 0; // position in line

  const char* const srcEnd = &src[src.size() - maskLen];
  const char* const maskEnd = mask + maskLen;
  for (const char* srcPtr = &src[0]; srcPtr != srcEnd; ++srcPtr)
  {
    if (*srcPtr == '\n')
    {
      ++line;
      linePos = 0;
      continue;
    }
    const char* srcCmp = srcPtr; // used as success flag in addition
    for (const char* maskPtr = mask; maskPtr < maskEnd; ++maskPtr)
    {
      if ((*maskPtr != '?' && *maskPtr != *srcCmp) ||
          //*srcCmp == '\n') // in order to avoid overlapping of '?' and '\n'
          !isprint(static_cast<unsigned char>(*srcCmp)))
      {
        srcCmp = nullptr;
        break;
      }
      ++srcCmp;
    }
    if (srcCmp)
    {
      HitItem hit {line, linePos, std::string(srcPtr, maskLen)};
      res.HitList.push_back(hit);
    }
    ++linePos;
  }
  res.EndPnt_Line = line;
  res.EndPnt_Pos = linePos;
  return res;
}

void executer(DataSupp& dataSupp, ResultProcessor& resultProcessor, const char* searchMask)
{
  const uint32_t maskLen = std::strlen(searchMask);
  std::list<ChunkResult> resList;
  DataChunk dataChunk;
  using Clock = std::chrono::high_resolution_clock;
  std::chrono::duration<double> readTime(0);
  std::chrono::duration<double> searchTime(0);
  while (true)
  {
    std::chrono::time_point<Clock> begTP = Clock::now();
    bool readRes = dataSupp.GetDataChunk(dataChunk);
    readTime += (Clock::now() - begTP);
    if (!readRes)
    {
      break;
    }
    begTP = Clock::now();
    ChunkResult chunkRes = ProcChunk(dataChunk.Data, searchMask, maskLen);
    searchTime += (Clock::now() - begTP);
    chunkRes.ChunkIdx = dataChunk.ChunkIdx;
    resList.emplace_back(std::move(chunkRes));
  }
  resultProcessor.AddChunkResults(std::move(resList));
  ExecutersTimers.AddExecuterTime({readTime, searchTime});
}



int main(int argc, char *argv[])
{
  if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))
  {
    std::cout << "Usage:\n  " << argv[0] << " <path/>file_name \"search_mask\"" << std::endl;
    return 0;
  }

  if (argc != 3)
  {
    std::cout << "Fault to run\n" << "Usage:\n  " << argv[0] << " <path/>file_name \"search_mask\"" << std::endl;
    return 0;
  }

  const char* srcFile = argv[1];
  const char* mask = argv[2];
  const size_t maskLen = std::strlen(mask);
  if (maskLen == 0 || maskLen > 100)
  {
    std::cout << "Mask length should be between 1 and 100 character\n" << "Usage:\n  " << argv[0] << " <path/>file_name \"search_mask\"" << std::endl;
    return 0;
  }

  using Clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<Clock> begTP = Clock::now();

  std::unique_ptr<ResultProcessor> resultProcessor(new ResultProcessor());
  try
  {
    {
      std::unique_ptr<DataSupp> dataSupp(new DataSupp(maskLen - 1, srcFile));
      std::vector<std::unique_ptr<std::thread, void(*)(std::thread*)>> listOfThreads;
      for (int i = 0; i < NUM_OF_CALC_THREADS; ++i)
      {
        listOfThreads.emplace_back(new std::thread(executer, std::ref(*dataSupp), std::ref(*resultProcessor), mask),
                                   [](std::thread* pThread) { if (pThread->joinable()) { pThread->join();} delete pThread; });
      }
    }
    std::cout << ExecutersTimers.GetInfo() << std::endl;
    std::chrono::time_point<Clock> postProcTP = Clock::now();
    resultProcessor->PostProcessing();
    std::cout << "PostProc time: " << std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - postProcTP).count() << " us" << std::endl;
    std::cout << resultProcessor->PrintResult().c_str() << std::endl;
  }

  catch (const std::exception& e)
  {
    std::cout << e.what();
  }
  catch (const char* e)
  {
    std::cout << e << std::endl;
  }
  std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - begTP).count() << " us" << std::endl;

  return 0;
}
