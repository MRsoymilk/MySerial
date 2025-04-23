// UILogSink.h
#ifndef UILOGSINK_H
#define UILOGSINK_H

#include <QMetaObject>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <mutex>
#include <spdlog/sinks/base_sink.h>

template<typename Mutex>
class UILogSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    UILogSink(QPlainTextEdit *widget)
        : widget_(widget)
    {}

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override
    {
        // 格式化为字符串
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);
        QString qtext = QString::fromStdString(fmt::to_string(formatted));

        // UI 更新必须在主线程
        if (widget_) {
            QMetaObject::invokeMethod(
                widget_,
                [=]() {
                    widget_->appendPlainText(qtext);
                    widget_->moveCursor(QTextCursor::End);
                },
                Qt::QueuedConnection);
        }
    }

    void flush_() override {}

private:
    QPlainTextEdit *widget_;
};

using UILogSink_mt = UILogSink<std::mutex>;

#endif // UILOGSINK_H
