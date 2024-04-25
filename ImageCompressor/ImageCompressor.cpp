#include "ImageCompressor.h"

using namespace::ImageCompressor;

namespace
{
enum class PixelColor
{
    WHITE = 0xff,
    BLACK = 0x00
};

enum class DataIdentifiers
{
    WHITE_IN_RAW = 0x00,
    BLACK_IN_RAW = 0x80,
    DIFFERENT = 0xC0,
    UNKNOWN
};

class BinaryWriter
{
public:
    BinaryWriter(): currentWriteIndex{7} {}
    void writeData(const BYTE*begin, int numOfBits)
    {
        if(begin)
        {
            int bitToCopy = 7;
            while(numOfBits > 0)
            {
                if(currentWriteIndex == 7)
                {
                    data.push_back(0x00);
                }

                data.back() |= ((*begin >> bitToCopy) & 0x01) << currentWriteIndex;

                --bitToCopy;
                --currentWriteIndex;
                --numOfBits;

                if(currentWriteIndex < 0)
                {
                    currentWriteIndex = 7;
                }

                if(bitToCopy < 0)
                {
                    ++begin;
                    bitToCopy = 7;
                }
            }
        }
    }
    std::vector<BYTE> getData()
    {
        return data;
    }

private:
    std::vector<BYTE> data;
    int currentWriteIndex;
};

class BinaryReader
{
public:
    BinaryReader(const std::vector<BYTE>& data): data{data}, readBit{7}, readIndex{0} {}

    BYTE readNextBit()
    {
        BYTE val = 0x00;

        if(!eof())
        {
            val = (data[readIndex] >> readBit & 0x01) << 7;

            readBit--;

            if(readBit<0)
            {
                readIndex++;
                readBit = 7;
            }
        }

        return val;
    }

    BYTE readNextByte()
    {
        BYTE val = 0x00;

        int numBitsToRead = 8;
        int bitToWrite = 7;

        while(!eof() && numBitsToRead>0)
        {
            val |= (data[readIndex] >> readBit & 0x01) << bitToWrite;

            --readBit;
            --numBitsToRead;
            --bitToWrite;

            if(readBit<0)
            {
                readIndex++;
                readBit = 7;
            }
        }

        return val;
    }

    bool eof() {return readIndex == data.size();}
private:
    const std::vector<BYTE>& data;
    int readBit;
    int readIndex;
};

//compression helpers
bool isEmptyRaw(const unsigned char*begin, const unsigned char*end)
{

    while(begin<end)
    {
        if(*begin != static_cast<unsigned char>(PixelColor::WHITE))
        {
            return false;
        }

        ++begin;
    }

    return true;
}

void writeWithIdentifier(DataIdentifiers identifier, BinaryWriter& binaryData, BYTE* begin = nullptr, ImageCompressor::BYTE* end = nullptr)
{
    BYTE ident = static_cast<BYTE>(identifier);

    switch(identifier)
    {
    case DataIdentifiers::BLACK_IN_RAW:
    {
        binaryData.writeData(&ident, 2);
        break;
    }
    case DataIdentifiers::WHITE_IN_RAW:
    {
        binaryData.writeData(&ident, 1);
        break;
    }
    case DataIdentifiers::DIFFERENT:
    {
        binaryData.writeData(&ident, 2);

        if(begin && end && begin < end)
        {
            int numOfBitsToWrite = (end - begin) * 8;
            binaryData.writeData(begin, numOfBitsToWrite);
        }

        break;
    }
    }
}

//decompression helpers
DataIdentifiers readNextCommand(BinaryReader& reader)
{
    if(reader.eof())
    {
        return DataIdentifiers::UNKNOWN;
    }

    BYTE command = reader.readNextBit();

    if(static_cast<BYTE>(DataIdentifiers::WHITE_IN_RAW) == command)
    {
        return DataIdentifiers::WHITE_IN_RAW;
    }
    else
    {
        if(reader.eof())
        {
            return DataIdentifiers::UNKNOWN;
        }

        command|=reader.readNextBit()>>1;

        if(static_cast<BYTE>(DataIdentifiers::BLACK_IN_RAW) == command)
        {
            return DataIdentifiers::BLACK_IN_RAW;
        }
        else if(static_cast<BYTE>(DataIdentifiers::DIFFERENT) == command)
        {
            return DataIdentifiers::DIFFERENT;
        }
        else
        {
            return DataIdentifiers::UNKNOWN;
        }
    }
}
}

ImageCompressor::CompressedImage ImageCompressor::compressImage(const RawImageData data)
{
    BinaryWriter binaryData;
    std::vector<bool> indexes;

    for(int raw = 0; raw<data.height; ++raw)
    {
        if(isEmptyRaw(data.data + raw * data.width, data.data + (raw + 1) * data.width))
        {
            indexes.push_back(0);
        }
        else
        {
            indexes.push_back(1);
            int samePixelsInRaw = 0;
            int startPixelIndex = raw * data.width;
            unsigned char startPixelData = data.data[startPixelIndex];

            int endOfColumn = (raw+1) * data.width;
            for(int column = startPixelIndex; column < endOfColumn;)
            {
                if(data.data[column] == startPixelData)
                {
                    ++samePixelsInRaw;

                    if(samePixelsInRaw == 4)
                    {
                        samePixelsInRaw = 0;

                        if(startPixelData == static_cast<BYTE>(PixelColor::BLACK))
                        {
                            writeWithIdentifier(DataIdentifiers::BLACK_IN_RAW, binaryData);
                        }
                        else if(startPixelData == static_cast<BYTE>(PixelColor::WHITE))
                        {
                            writeWithIdentifier(DataIdentifiers::WHITE_IN_RAW, binaryData);
                        }
                        else
                        {
                            writeWithIdentifier(DataIdentifiers::DIFFERENT, binaryData, &data.data[startPixelIndex], &data.data[startPixelIndex] + 4);
                        }

                        startPixelIndex+=4;
                        startPixelData = data.data[startPixelIndex];
                        column = startPixelIndex;
                        continue;
                    }
                }
                else
                {
                    samePixelsInRaw = 0;
                    int endCopyIndex = (startPixelIndex + 4) <= endOfColumn ? startPixelIndex + 4 : endOfColumn;
                    int diff = endCopyIndex - startPixelIndex;
                    writeWithIdentifier(DataIdentifiers::DIFFERENT, binaryData, &data.data[startPixelIndex], &data.data[startPixelIndex] + diff);
                    startPixelIndex = endCopyIndex;
                    startPixelData = data.data[startPixelIndex];
                    column = startPixelIndex;
                    continue;
                }

                if(column == endOfColumn - 1)
                {
                    int diff = endOfColumn - startPixelIndex;
                    writeWithIdentifier(DataIdentifiers::DIFFERENT, binaryData, &data.data[startPixelIndex], &data.data[startPixelIndex] + diff);
                }

                ++column;
            }
        }
    }

    CompressedImage compressed;
    compressed.width = data.width;
    compressed.height = data.height;
    compressed.compressedIndexes = std::move(indexes);
    compressed.data = binaryData.getData();

    return compressed;
}

ImageCompressor::RawImageData ImageCompressor::decompressImage(const CompressedImage data)
{
    RawImageData imageData;

    int decompressedBytes = 0;

    std::vector<BYTE> decompressedData;
    BinaryReader reader(data.data);

    for(auto index : data.compressedIndexes)
    {
        if(index == 0)
        {
            decompressedData.insert(decompressedData.end(), data.width, static_cast<BYTE>(PixelColor::WHITE));
            decompressedBytes += data.width;
        }
        else
        {
            int readRawBytes = 0;

            while(!reader.eof() && readRawBytes<data.width)
            {
                auto command = readNextCommand(reader);

                if(command == DataIdentifiers::UNKNOWN)
                {
                    throw ImageCompressorException(ExceptionType::INCORRECT_DATA_IN_DECOMPRESSION);
                }
                else if(command == DataIdentifiers::BLACK_IN_RAW)
                {
                    decompressedData.insert(decompressedData.end(), 4, static_cast<BYTE>(PixelColor::BLACK));
                    readRawBytes += 4;
                }
                else if(command == DataIdentifiers::WHITE_IN_RAW)
                {
                    decompressedData.insert(decompressedData.end(), 4, static_cast<BYTE>(PixelColor::WHITE));
                    readRawBytes += 4;
                }
                else if(command == DataIdentifiers::DIFFERENT)
                {
                    int numBytesToRead = (data.width - readRawBytes) < 4 ? data.width - readRawBytes : 4 ;

                    while(!reader.eof() && numBytesToRead>0)
                    {
                        decompressedData.push_back(reader.readNextByte());

                        --numBytesToRead;
                        readRawBytes += 1;
                    }
                }
            }

            decompressedBytes += readRawBytes;
        }
    }

    if(decompressedBytes < data.width*data.height)
    {
        throw ImageCompressorException(ExceptionType::INCORRECT_DATA_IN_DECOMPRESSION);
    }

    imageData.height = data.height;
    imageData.width = data.width;
    imageData.data = new BYTE[decompressedData.size()];
    memcpy(imageData.data, decompressedData.data(), decompressedData.size());

    return imageData;
}
