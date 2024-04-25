#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QVector>
#include <QPair>
#include <QString>
#include <QRgb>
#include <QImage>
#include "FilesModel.h"
#include "ImageCompressor.h"

struct RecoveryImageData
{
    int32_t originalImageWidth;
    QVector<QRgb> colorTable;
    QImage::Format format;
};

struct OriginalImageData
{
    ImageCompressor::RawImageData data;
    RecoveryImageData recoveryData;
};

struct CompressedImageData
{
    ImageCompressor::CompressedImage data;
    RecoveryImageData recoveryData;
    bool isValid = false;
};

class ImageHandler : public QObject
{
    Q_OBJECT
public:
    explicit ImageHandler(FilesModel& model, QObject *parent = nullptr);

public slots:
    void onClickFile(int index);

signals:
    void error(const QString error);

private:
    void changeFileStatus(const QString& filepath, FileInfo::FileStatus status);
    void onDecompressionFinished(OriginalImageData& decompressed, const QString& path);
    void onCompressionFinished(CompressedImageData& compressed, const QString& path);

    OriginalImageData getImageDataFromImage(const QString& path);
    CompressedImageData getCompressedImageDataFromFile(const QString& path);

private:
    FilesModel& model;
};

#endif // IMAGEHANDLER_H
