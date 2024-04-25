
#include "ImageHandler.h"
#include <QtConcurrent>
#include <QBitmap>
#include <QImage>
#include <QFile>

ImageHandler::ImageHandler(FilesModel &model, QObject *parent)
    :model{model}
{

}

void ImageHandler::onClickFile(int index)
{
    QModelIndex modelInd = model.index(index);
    QString path = model.data(modelInd, static_cast<int>(FilesModel::FileRoles::FILENAME_ROLE)).toString();
    QFileInfo file(path);

    if(file.suffix() == "bmp")
    {
        OriginalImageData originalData = getImageDataFromImage(path);

        if(originalData.data.data)
        {
            QFutureWatcher<ImageCompressor::CompressedImage>* watcher = new QFutureWatcher<ImageCompressor::CompressedImage>();

            connect(watcher, &QFutureWatcher<void>::finished, [=](){
                ImageCompressor::CompressedImage result = watcher->result();
                CompressedImageData compressedData;
                compressedData.data = std::move(result);
                compressedData.recoveryData = std::move(originalData.recoveryData);
                onCompressionFinished(compressedData, path);
                delete originalData.data.data;
                changeFileStatus(path, FileInfo::FileStatus::NONE);
                watcher->deleteLater();
            });

            watcher->setFuture(QtConcurrent::run(ImageCompressor::compressImage, originalData.data));

            model.setData(modelInd, QVariant(static_cast<int>(FileInfo::FileStatus::COMPRESSING)), static_cast<int>(FilesModel::FileRoles::STATUS_ROLE));
        }
    }
    else if(file.suffix() == "barch")
    {
        CompressedImageData compressedData = getCompressedImageDataFromFile(path);

        if(compressedData.isValid)
        {
            QFutureWatcher<ImageCompressor::RawImageData>* watcher = new QFutureWatcher<ImageCompressor::RawImageData>();

            connect(watcher, &QFutureWatcher<void>::finished, [=](){
                ImageCompressor::RawImageData result = watcher->result();
                OriginalImageData originalData;
                originalData.data = std::move(result);
                originalData.recoveryData = std::move(compressedData.recoveryData);
                onDecompressionFinished(originalData, path);
                delete result.data;
                changeFileStatus(path, FileInfo::FileStatus::NONE);
                watcher->deleteLater();
            });

            watcher->setFuture(QtConcurrent::run(ImageCompressor::decompressImage, compressedData.data));

            model.setData(modelInd, QVariant(static_cast<int>(FileInfo::FileStatus::DECOMPRESSING)), static_cast<int>(FilesModel::FileRoles::STATUS_ROLE));
        }
    }
    else
    {
        emit error("Incorrect file extension. Use only .bmp or .barch!");
    }
}

void ImageHandler::onCompressionFinished(CompressedImageData& compressed, const QString& path)
{
    QString newPath = path;
    QString removeExtension = ".bmp";
    newPath.remove(newPath.lastIndexOf(removeExtension), removeExtension.size());
    newPath+="_packed.barch";

    QFile newFile(newPath);
    newFile.open(QFile::WriteOnly);

    if(newFile.isOpen())
    {
        newFile.write((char*)&compressed.recoveryData.format, sizeof(QImage::Format));
        newFile.write((char*)&compressed.recoveryData.originalImageWidth, sizeof(qint32));
        int size = compressed.recoveryData.colorTable.size();
        newFile.write((char*)&size, sizeof(qint32));
        newFile.write((char*)compressed.recoveryData.colorTable.data(), sizeof(QRgb) * compressed.recoveryData.colorTable.size());

        newFile.write((char*)&compressed.data.width, sizeof(qint32));
        newFile.write((char*)&compressed.data.height, sizeof(qint32));
        size = compressed.data.compressedIndexes.size();
        newFile.write((char*)&size, sizeof(qint32));

        for(bool el : compressed.data.compressedIndexes)
        {
            newFile.write((char*)&el, sizeof(bool));
        }

        size = compressed.data.data.size();
        newFile.write((char*)&size, sizeof(qint32));
        newFile.write((char*)compressed.data.data.data(), size);

        newFile.close();
    }
    else
    {
        emit error("File can't be opened: " + newPath);
    }
}

OriginalImageData ImageHandler::getImageDataFromImage(const QString &path)
{
    OriginalImageData data;

    QImage image(path);

    if(!image.isNull())
    {
        ImageCompressor::RawImageData rawData;

        rawData.height = image.height();
        rawData.width = image.bytesPerLine();

        rawData.data = new unsigned char[image.sizeInBytes()];
        memcpy(rawData.data, image.bits(), image.sizeInBytes());

        data.data = rawData;
        data.recoveryData.originalImageWidth = image.width();
        data.recoveryData.colorTable = image.colorTable();
        data.recoveryData.format = image.format();
    }
    else
    {
        emit error("File can't be opened: " + path);
    }

    return data;
}

CompressedImageData ImageHandler::getCompressedImageDataFromFile(const QString &path)
{
    CompressedImageData data;

    QFile file(path);
    file.open(QFile::ReadOnly);
    bool noError = true;

    if(file.isOpen())
    {
        int size=0;

        noError&=file.read((char*)&data.recoveryData.format, sizeof(QImage::Format)) == sizeof(QImage::Format);
        noError&=file.read((char*)&data.recoveryData.originalImageWidth, sizeof(qint32)) == sizeof(qint32);
        noError&=file.read((char*)&size, sizeof(qint32)) == sizeof(qint32);

        for(int i = 0; i<size; ++i)
        {
            QRgb val = 0;
            noError&=file.read((char*)&val, sizeof(QRgb)) == sizeof(QRgb);

            data.recoveryData.colorTable.push_back(val);
        }

        size=0;
        noError&=file.read((char*)&data.data.width, sizeof(qint32)) == sizeof(qint32);
        noError&=file.read((char*)&data.data.height, sizeof(qint32)) == sizeof(qint32);
        noError&=file.read((char*)&size, sizeof(qint32)) == sizeof(qint32);

        data.data.compressedIndexes.reserve(size);

        for(int i = 0; i<size; ++i)
        {
            bool hasItems = false;
            noError&=file.read((char*)&hasItems, sizeof(bool)) == sizeof(bool);

            data.data.compressedIndexes.push_back(hasItems);
        }

        size = 0;
        noError&=file.read((char*)&size, sizeof(qint32)) == sizeof(qint32);
        data.data.data.reserve(size);

        for(int i = 0; i<size; ++i)
        {
            unsigned char symbol = 0;
            noError&=file.read((char*)&symbol, sizeof(char)) == sizeof(char);

            data.data.data.push_back(symbol);
        }

        file.close();

        if(noError == false)
        {
            emit error("Incorrect file data: " + path);
        }
    }
    else
    {
        emit error("File can't be opened: " + path);
    }

    data.isValid = noError;

    return data;
}

void ImageHandler::onDecompressionFinished(OriginalImageData& decompressed, const QString& path)
{
    QString newPath = path;
    QString removeExtension = ".barch";
    newPath.remove(newPath.lastIndexOf(removeExtension), removeExtension.size());
    newPath+="_unpacked.bmp";

    QImage image(decompressed.data.data, decompressed.recoveryData.originalImageWidth, decompressed.data.height, decompressed.recoveryData.format);
    image.setColorTable(decompressed.recoveryData.colorTable);

    if(!image.save(newPath))
    {
        emit error("Error on save to: " + newPath);
    }
}

void ImageHandler::changeFileStatus(const QString &filepath, FileInfo::FileStatus status)
{
    QModelIndex ind = model.getModelIndexByFile(filepath);

    if(ind.isValid())
    {
        model.setData(ind, QVariant(static_cast<int>(status)), static_cast<int>(FilesModel::FileRoles::STATUS_ROLE));
    }
}
