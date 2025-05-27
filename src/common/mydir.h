#ifndef MYDIR_H
#define MYDIR_H

#include <QString>

class MyDir
{
   public:
    MyDir() = default;
    ~MyDir() = default;
    static bool CheckDirExists(const QString &path);
    static bool CreateDir(const QString &path);
    static std::string Path2Path(const QString &path);
    static QString GetAbsolutePath(const QString &relativePath);
};

#endif  // MYDIR_H
