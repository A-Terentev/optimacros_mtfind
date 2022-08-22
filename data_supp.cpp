#include "data_supp.h"
#include <chrono>
#include <iostream>
#include <cstdio>

#define MIN_CHUNK_NUM 4
#define MAX_CHUNK_SIZE 8192

DataSupp::DataSupp(const size_t aIntersectionSize, const char* aFileName) : intersectionSize(aIntersectionSize),
  file(std::fopen(aFileName, "rb"), &std::fclose)
{
  if (!file)
  {
    throw "Can't open source file";
  }
  // Get file size
  fseek(file.get(), 0, SEEK_END);
  fileSize = ftell(file.get());
  rewind(file.get());
  // Calc chunk size
  chunkSize = fileSize / MIN_CHUNK_NUM;
  if (chunkSize == 0)
  {
    throw "File too small";
  }
  if (chunkSize > MAX_CHUNK_SIZE)
  {
    chunkSize = MAX_CHUNK_SIZE;
  }
  intersectionData.reserve(aIntersectionSize);
}

DataSupp::~DataSupp()
{
  ;
}

bool DataSupp::GetDataChunk(DataChunk& aDataChunk)
{
  aDataChunk.Data.clear();
  aDataChunk.Data.reserve(chunkSize + intersectionSize);

  std::lock_guard<std::mutex> lock(dataMutex);
  size_t sizeToRead = fileSize - curFilePos;
  sizeToRead = (sizeToRead > chunkSize ? chunkSize : sizeToRead);
  const size_t intersectionOffset = intersectionData.size();
  aDataChunk.ChunkIdx = chunkIdx++;
  if (intersectionOffset)
  {
    aDataChunk.Data.insert(aDataChunk.Data.begin(), intersectionData.begin(), intersectionData.end());
    intersectionData.clear();
  }
  if (sizeToRead)
  {
    aDataChunk.Data.resize(aDataChunk.Data.size() + sizeToRead);
    const size_t res = std::fread(&(aDataChunk.Data[intersectionOffset]), sizeof(char), sizeToRead, file.get());
    if (res != sizeToRead)
    {
      throw "Read file error";
    }

    if (aDataChunk.Data.size() > intersectionSize)
    {
      intersectionData.assign(aDataChunk.Data.cend() - intersectionSize, aDataChunk.Data.cend());
    }
    curFilePos += sizeToRead;
  }

  return aDataChunk.Data.size() > intersectionSize;
}
