// #ifndef THREADPARSER_H
// #define THREADPARSER_H

// #include <QObject>
// #include "global.h"

// class ThreadParser : public QObject
// {
//     Q_OBJECT

// public:
//     explicit ThreadParser(QObject *parent = nullptr);
//     void setFrameTypes(const QList<FrameType> &types);

// public slots:
//     void pushData(const QByteArray &data);

// signals:
//     void frameParsed(const QString &type, const QByteArray &frame, const QByteArray &temp="");

// private:
//     QList<FrameType> m_frameTypes;
//     QByteArray m_buffer;

//     void parse();
// };

// #endif // THREADPARSER_H
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>

#include "global.h"

class ThreadParser : public QObject {
    Q_OBJECT

public:
    explicit ThreadParser(QObject *parent = nullptr);

    void setFrameTypes(const QList<FrameType> &types);

public slots:
    void pushData(const QByteArray &data);

signals:
    void frameParsed(const QString &name, const QByteArray &frame, const QByteArray &temp = "");

private:
    void parse();

private:
    QByteArray m_buffer;
    QList<FrameType> m_frameTypes;

    QMutex m_mutex;
};
