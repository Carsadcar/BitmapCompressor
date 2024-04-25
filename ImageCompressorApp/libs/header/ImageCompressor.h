#ifndef IMAGECOMPRESSOR_H
#define IMAGECOMPRESSOR_H

#include <vector>
#include <memory>

namespace ImageCompressor
{
    using BYTE = unsigned char;

    struct RawImageData {
        int width = 0; // image width in pixels
        int height = 0; // image height in pixels
        unsigned char *data = nullptr; // Pointer to image data. data[j * width + i] is color of pixel in row j and column i.
    };

    struct CompressedImage
    {
        int width = 0; // image width in pixels
        int height = 0; // image height in pixels
        std::vector<bool> compressedIndexes;
        std::vector<BYTE> data;
    };

    enum class ExceptionType
    {
        INCORRECT_DATA_IN_DECOMPRESSION = 0
    };

    class ImageCompressorException : public std::exception
    {
    public:
        ImageCompressorException(ExceptionType type) : exceptionType{type} {}
        const char* what() const _GLIBCXX_USE_NOEXCEPT override
        {
            std::string exceptionData{"Exception on "};

            switch(exceptionType)
            {
            case ExceptionType::INCORRECT_DATA_IN_DECOMPRESSION:
            {
            exceptionData+= "decompression. Incorrect size of compressing data.";
            break;
            }
            }

            return exceptionData.c_str();
        }
    private:
        ExceptionType exceptionType;
    };

    CompressedImage compressImage(const RawImageData data);
    RawImageData decompressImage(const CompressedImage data);
};

#endif // IMAGECOMPRESSOR_H
