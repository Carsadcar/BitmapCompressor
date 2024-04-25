#ifndef FILEINFO_H
#define FILEINFO_H

#include <string>
#include <QString>

class FileInfo
{
public:
    enum class FileStatus
    {
        NONE = 0,
        COMPRESSING,
        DECOMPRESSING
    };

public:
    FileInfo(const QString& filePath, uint64_t size) : filePath{filePath}, size{size}, status{FileStatus::NONE} {}
    void setStatus(FileStatus status) {this->status = status;}
    FileStatus getStatus(){return status;}
    uint64_t getSize() const {return size;}
    QString getStatusString() const
    {
        QString statusStr;

        switch(status)
        {
        case FileStatus::COMPRESSING:
        {
            statusStr+="compressing";
            break;
        }
        case FileStatus::DECOMPRESSING:
        {
            statusStr+="decompressing";
            break;
        }
        }

        return statusStr;
    }
    QString getFilePath() const {return filePath;}

private:
    QString filePath;
    FileStatus status;
    uint64_t size;
};

#endif // FILEINFO_H
