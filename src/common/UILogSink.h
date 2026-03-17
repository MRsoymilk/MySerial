#ifndef UILOGSINK_H
#define UILOGSINK_H

#include <spdlog/sinks/base_sink.h>

#include <QMetaObject>
#include <QPlainTextEdit>
#include <QPointer>
#include <QTextCursor>
#include <mutex>

template <typename Mutex>
class UILogSink : public spdlog::sinks::base_sink<Mutex> {
public:
    UILogSink(QPlainTextEdit *widget) : widget_(widget) {}

    void setWidget(QPlainTextEdit *widget) { widget_ = widget; }

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);

        QString qtext = QString::fromStdString(fmt::to_string(formatted));

        if (!widget_) return;

        QPointer<QPlainTextEdit> w = widget_;

        QMetaObject::invokeMethod(
            widget_,
            [w, qtext]() {
                if (!w) return;

                w->appendPlainText(qtext);
                w->moveCursor(QTextCursor::End);
            },
            Qt::QueuedConnection);
    }

    void flush_() override {}

private:
    QPointer<QPlainTextEdit> widget_;
};

using UILogSink_mt = UILogSink<std::mutex>;

#endif
