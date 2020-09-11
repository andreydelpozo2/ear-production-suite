#include "pcmwriter.h"
#include "pcmreader.h"
#include "bw64/bw64.hpp"

using namespace admplug;

PCMWriter::PCMWriter(std::string name) : name{name}
{

}

PCMWriter::~PCMWriter() = default;

void PCMWriter::write(const IPCMBlock &block)
{
    if(!writer) {
        writer = bw64::writeFile(fileName().c_str(), block.channelCount(), block.sampleRate(), 24);
    }
    auto framesWritten = writer->write(&block.data()[0], block.frameCount());
}

std::string PCMWriter::fileName()
{
    return name;
}
