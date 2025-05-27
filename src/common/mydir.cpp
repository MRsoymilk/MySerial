#include "mydir.h"

#include <QDir>

bool MyDir::CheckDirExists(const QString &path)
{
    QDir dir(path);
    return dir.exists();
}

bool MyDir::CreateDir(const QString &path)
{
    QDir dir;
    return dir.mkpath(path);
}

std::string MyDir::Path2Path(const QString &path)
{
    QByteArray byteFilePath = path.toLocal8Bit();
    std::string stdFilePath = byteFilePath.toStdString();
    return stdFilePath;
}

QString MyDir::GetAbsolutePath(const QString &relativePath)
{
    QFileInfo fileInfo(relativePath);
    QString absolutePath = fileInfo.absoluteFilePath();
    return absolutePath;
}
