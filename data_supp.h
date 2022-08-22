#ifndef DATA_SUPP_H
#define DATA_SUPP_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <memory>
#include <vector>
#include <set>

struct DataChunk
{
  uint32_t ChunkIdx;
  std::vector<char> Data;
};

class DataSupp
{
  using UniqFile = std::unique_ptr<std::FILE, decltype(&std::fclose)>;
  using ChunkSet = std::vector<DataChunk>;

public:
  DataSupp(const size_t aIntersectionSize, const char* aFileName);
  ~DataSupp();
  bool GetDataChunk(DataChunk& aDataChunk);

private:
  const size_t intersectionSize;
  UniqFile file;
  size_t fileSize;
  size_t curFilePos = 0;
  uint32_t chunkIdx = 0;
  size_t chunkSize;
  std::vector<char> intersectionData;
  std::mutex dataMutex;
};

#endif
