#ifndef FILESMODEL_H
#define FILESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QTimer>

#include "FileInfo.h"

class FilesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class FileRoles{
        FILENAME_ROLE = Qt::UserRole + 1,
        SIZE_ROLE,
        STATUS_ROLE
    };

public:
   explicit FilesModel(const QString& dirPath, QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QHash<int, QByteArray> roleNames() const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QModelIndex getModelIndexByFile(const QString& path);

private slots:
    void updateListFiles();

private:
    QList<FileInfo> files;
    QString directory;
    QTimer filesUpdater;
};

#endif // FILESMODEL_H
