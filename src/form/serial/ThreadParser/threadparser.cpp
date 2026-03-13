#include "threadparser.h"
#include "funcdef.h"

#include <QMutexLocker>

ThreadParser::ThreadParser(QObject *parent)
    : QObject(parent)
{
}

void ThreadParser::setFrameTypes(const QList<FrameType> &types)
{
    QMutexLocker locker(&m_mutex);
    m_frameTypes = types;
}

void ThreadParser::pushData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);

    m_buffer.append(data);

    // 防止 buffer 无限增长
    if (m_buffer.size() > 1024 * 1024) {
        LOG_WARN("Buffer too large, clearing");
        m_buffer.clear();
        return;
    }

    parse();
}

void ThreadParser::parse()
{
    while (true) {

        if (m_frameTypes.isEmpty()) {
            m_buffer.clear();
            return;
        }

        int firstHeaderIdx = -1;
        FrameType current;

        for (const auto &type : m_frameTypes) {
            int idx = m_buffer.indexOf(type.header);

            if (idx != -1 && (firstHeaderIdx == -1 || idx < firstHeaderIdx)) {
                firstHeaderIdx = idx;
                current = type;
            }
        }

        if (firstHeaderIdx == -1) {

            if (m_buffer.size() > 1024 * 10) {
                LOG_WARN("No header found, clearing buffer");
                m_buffer.clear();
            }

            return;
        }

        if (firstHeaderIdx > 0) {

            LOG_WARN("Dropping invalid data before header: {} bytes",
                     firstHeaderIdx);

            m_buffer.remove(0, firstHeaderIdx);
        }

        if (current.length != 0) {

            if (m_buffer.size() < current.length)
                return;

            QByteArray frame = m_buffer.left(current.length);

            if (!frame.endsWith(current.footer)) {

                LOG_WARN("Invalid footer (fixed length), resync");

                // 关键修复：只删除1字节
                m_buffer.remove(0, 1);

                continue;
            }

            LOG_INFO("Fixed-length frame matched: {}",
                     current.name.toStdString());

            emit frameParsed(current.name, frame);

            m_buffer.remove(0, current.length);

            continue;
        }

        // -------- 变长帧 --------
        int footerIdx =
            m_buffer.indexOf(current.footer,
                             current.header.size());

        if (footerIdx == -1)
            return;

        int frameLen = footerIdx + current.footer.size();

        QByteArray frame = m_buffer.left(frameLen);

        LOG_INFO("Variable-length frame matched: {}, size={}",
                 current.name.toStdString(),
                 frameLen);

        emit frameParsed(current.name, frame);

        m_buffer.remove(0, frameLen);
    }
}
