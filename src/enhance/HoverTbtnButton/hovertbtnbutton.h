#ifndef HOVERTBTNBUTTON_H
#define HOVERTBTNBUTTON_H

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QEnterEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QTimer>
#include <QToolButton>
#include <QWidget>

class HoverTbtnButton : public QToolButton {
    Q_OBJECT
public:
    enum H_BTN {
        BTN_HANDSHAKE,
        BTN_SET_INTEGRATION_TIME,
        BTN_DO_THRESHOLD,
        BTN_DO_BASELINE,
        BTN_DATA_REQUEST,
        BTN_STOP,
        BTN_REFRESH
    };

public:
    explicit HoverTbtnButton(QWidget *parent = nullptr) : QToolButton(parent) {
        setMouseTracking(true);

        m_popup = new QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        m_popup->setFocusPolicy(Qt::StrongFocus);
        m_popup->setFixedSize(360, 40);

        QHBoxLayout *layout = new QHBoxLayout(m_popup);
        layout->setContentsMargins(1, 1, 1, 1);
        layout->setSpacing(1);

        m_box = new QComboBox;
        layout->addWidget(m_box);

        addButton(layout, tr("handshake"), "HtBtnHandshake", BTN_HANDSHAKE);
        addButton(layout, tr("integration"), "HtBtnIntegration", BTN_SET_INTEGRATION_TIME);
        addButton(layout, tr("threshold"), "HtBtnThreshold", BTN_DO_THRESHOLD);
        addButton(layout, tr("baseline"), "HtBtnBaseline", BTN_DO_BASELINE);
        addButton(layout, tr("data request"), "HtBtnDataRequest", BTN_DATA_REQUEST);
        addButton(layout, tr("stop"), "HtBtnStop", BTN_STOP);
        addButton(layout, tr("refresh"), "HtBtnRefresh", BTN_REFRESH);

        m_popup->installEventFilter(this);
    }

    QString getCurrentPort() {
        QString port = m_box->currentText();
        return port;
    }

public slots:
    void updatePorts(const QStringList &ports) {
        m_box->clear();
        m_box->addItems(ports);
    }

signals:
    void buttonClicked(int id);
    void buttonHover();

protected:
    void enterEvent(QEnterEvent *event) override {
        Q_UNUSED(event);

        if (m_popup->isVisible()) {
            return;
        }

        QPoint btnGlobal = mapToGlobal(QPoint(0, 0));

        int x = btnGlobal.x();
        int y = btnGlobal.y() + height() + 4;

        QScreen *screen = QGuiApplication::screenAt(btnGlobal);
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }

        QRect screenRect = screen->availableGeometry();

        if (x < screenRect.left()) {
            x = screenRect.left();
        }

        if (x + m_popup->width() > screenRect.right()) {
            x = screenRect.right() - m_popup->width();
        }

        if (y + m_popup->height() > screenRect.bottom()) {
            y = btnGlobal.y() - m_popup->height() - 4;
        }

        if (y < screenRect.top()) {
            y = screenRect.top();
        }

        m_popup->move(x, y);
        m_popup->show();
        emit buttonHover();
    }

    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == m_popup) {
            if (event->type() == QEvent::Leave) {
                QTimer::singleShot(100, this, [=]() {
                    if (m_box->view()->isVisible()) {
                        return;
                    }
                    QWidget *under = QApplication::widgetAt(QCursor::pos());
                    if (under && (m_popup->isAncestorOf(under) || under == m_popup)) {
                        return;
                    }
                    m_popup->hide();
                });
            }
        }
        return QToolButton::eventFilter(obj, event);
    }

private:
    QWidget *m_popup;
    QComboBox *m_box;

private:
    void addButton(QHBoxLayout *layout, const QString &text, const QString &name, int id) {
        QToolButton *btn = new QToolButton;
        btn->setObjectName(name);
        btn->setText(text);
        btn->setToolTip(text);
        btn->setAutoRaise(true);

        layout->addWidget(btn);

        connect(btn, &QToolButton::clicked, this, [=]() { emit buttonClicked(id); });
    }
};

#endif  // HOVERTBTNBUTTON_H
