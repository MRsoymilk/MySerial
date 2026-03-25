#ifndef HOVERSLIDERBUTTON_H
#define HOVERSLIDERBUTTON_H

#include <QToolButton>
#include <QSlider>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QEnterEvent>
#include <QEvent>

class HoverSliderButton : public QToolButton
{
    Q_OBJECT
public:
    explicit HoverSliderButton(QWidget *parent = nullptr)
        : QToolButton(parent)
    {
        setMouseTracking(true);

        m_popup = new QWidget(nullptr, Qt::Popup);
        m_popup->setFixedSize(220, 60);

        QVBoxLayout *layout = new QVBoxLayout(m_popup);
        layout->setContentsMargins(1, 1, 1, 1);
        layout->setSpacing(2);

        m_label = new QLabel("0", m_popup);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setFixedHeight(18);

        layout->addWidget(m_label, 0, Qt::AlignHCenter);

        m_slider = new QSlider(Qt::Horizontal);
        m_slider->setRange(0, 100);
        m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        layout->addWidget(m_slider);

        connect(m_slider, &QSlider::valueChanged, this, [=](int v){

            m_label->setText(QString("%1 %").arg(v));

            int min = m_slider->minimum();
            int max = m_slider->maximum();

            double ratio = (v - min) * 1.0 / (max - min);

            int sliderWidth = m_slider->width();
            int handleX = ratio * sliderWidth;

            int labelWidth = m_label->width();

            int x = handleX - labelWidth / 2;

            x = qMax(0, qMin(x, sliderWidth - labelWidth));

            m_label->move(x, m_label->y());

            emit valueChanged(v);
        });

        m_popup->installEventFilter(this);
    }

signals:
    void valueChanged(int v);

protected:
    void enterEvent(QEnterEvent *event) override
    {
        Q_UNUSED(event);

        if (m_popup->isVisible())
            return;

        QPoint btnGlobal = mapToGlobal(QPoint(0, 0));

        int x = btnGlobal.x() + width()/2 - m_popup->width()/2;
        int y = btnGlobal.y() + height() + 4;

        m_popup->move(x, y);
        m_popup->show();
    }

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == m_popup) {
            if (event->type() == QEvent::Leave) {
                m_popup->hide();
            }
        }
        return QToolButton::eventFilter(obj, event);
    }

private:
    QWidget *m_popup;
    QSlider *m_slider;
    QLabel *m_label;
};

#endif // HOVERSLIDERBUTTON_H
