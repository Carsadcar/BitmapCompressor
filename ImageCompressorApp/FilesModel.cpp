#include "FilesModel.h"

#include <QDir>
#include <algorithm>

FilesModel::FilesModel(const QString& dirPath, QObject *parent) : directory{dirPath}
{
    updateListFiles();

    connect(&filesUpdater, &QTimer::timeout, this, &FilesModel::updateListFiles);
    filesUpdater.setInterval(1000);
    filesUpdater.start();
}

int FilesModel::rowCount(const QModelIndex &parent) const
{
    return files.size();
}

QVariant FilesModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= files.size())
        return QVariant();
    //The index is valid
    const FileInfo& file = files[index.row()];
    if( static_cast<FileRoles>(role) == FileRoles::FILENAME_ROLE)
        return file.getFilePath();
    if( static_cast<FileRoles>(role) == FileRoles::STATUS_ROLE)
        return file.getStatusString();
    if( static_cast<FileRoles>(role) == FileRoles::SIZE_ROLE)
        return file.getSize();
     return QVariant();
}

bool FilesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= files.size())
        return false;

    bool somethingChanged = false;
    FileRoles fileRole = static_cast<FileRoles>(role);
    FileInfo& file = files[index.row()];

    switch(fileRole)
    {
    case FileRoles::STATUS_ROLE:
    {
        if(file.getStatus() != static_cast<FileInfo::FileStatus>(value.toInt()))
        {
            file.setStatus(static_cast<FileInfo::FileStatus>(value.toInt()));
            somethingChanged = true;
        }
        break;
    }
    }

    if(somethingChanged)
    {
        emit dataChanged(index,index,QVector<int>());
        return true;
    }

    return false;
}

QHash<int, QByteArray> FilesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[static_cast<int>(FileRoles::FILENAME_ROLE)] = "filename";
    roles[static_cast<int>(FileRoles::SIZE_ROLE)] = "size";
    roles[static_cast<int>(FileRoles::STATUS_ROLE)] = "status";
    return roles;
}

Qt::ItemFlags FilesModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable;
}

QModelIndex FilesModel::getModelIndexByFile(const QString &path)
{
    for(int i = 0; i<files.size(); ++i)
    {
        if(files[i].getFilePath() == path)
        {
            return index(i);
        }
    }

    return {};
}

void FilesModel::updateListFiles()
{
    QDir dir(directory, "*.bmp *.barch *.png", QDir::Name | QDir::Type, QDir::Files);

    auto list = dir.entryInfoList();

    auto currentFilesIndex = 0;

    for(auto& info : list)
    {
        if(currentFilesIndex >= files.size())
        {
            beginInsertRows(QModelIndex(),currentFilesIndex,currentFilesIndex);
            files.append(FileInfo(info.absoluteFilePath(), info.size()));
            endInsertRows();
        }
        else if(files[currentFilesIndex].getFilePath() != info.absoluteFilePath())
        {
            auto it = std::find_if(files.begin()+currentFilesIndex, files.end(), [path = info.absoluteFilePath()](const FileInfo& file){return path == file.getFilePath();});

            if(it == files.end())
            {
                beginInsertRows(QModelIndex(),currentFilesIndex,currentFilesIndex);
                files.insert(files.begin()+currentFilesIndex, FileInfo(info.absoluteFilePath(), info.size()));
                endInsertRows();
            }
            else
            {
                int indexEndErase = it - files.begin();

                beginRemoveRows(QModelIndex(), currentFilesIndex, indexEndErase - 1);
                files.erase(files.begin() + currentFilesIndex, files.begin() + indexEndErase);
                endRemoveRows();
            }
        }

        currentFilesIndex++;
    }

    if(files.begin() + currentFilesIndex < files.end())
    {
        beginRemoveRows(QModelIndex(), currentFilesIndex, files.size() - 1);
        files.erase(files.begin() + currentFilesIndex, files.end());
        endRemoveRows();
    }
}
