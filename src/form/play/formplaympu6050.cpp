#include "formplaympu6050.h"
#include "ui_formplaympu6050.h"

#include <QFile>

#include <QPainter>
#include <QtMath>

EulerWidget::EulerWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(MIN_SIZE, MIN_SIZE);
    updateCachedBackground();
}

void EulerWidget::setEuler(float roll, float pitch, float yaw)
{
    // 限制欧拉角范围
    m_roll = qBound(-180.0f, roll, 180.0f);
    m_pitch = qBound(-90.0f, pitch, 90.0f);
    m_yaw = fmod(yaw, 360.0f);
    if (m_yaw < 0)
        m_yaw += 360.0f;
    update();
}

void EulerWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 绘制缓存的背景
    p.drawPixmap(0, 0, m_background);

    // 计算仪表盘区域（基于窗口高度）
    int h = height();
    int rollRadius = h * ROLL_RADIUS_RATIO;
    int pitchHeight = h * PITCH_HEIGHT_RATIO;
    int yawRadius = h * YAW_RADIUS_RATIO;
    int spacing = h * SPACING_RATIO;

    // 确保表盘中心相对于窗口中心
    QRect rollRect(-(rollRadius + BORDER_WIDTH),
                   -spacing - rollRadius - BORDER_WIDTH,
                   (rollRadius + BORDER_WIDTH) * 2,
                   (rollRadius + BORDER_WIDTH) * 2);
    QRect pitchRect(-40 - BORDER_WIDTH,
                    -pitchHeight / 2 - BORDER_WIDTH,
                    80 + 2 * BORDER_WIDTH,
                    pitchHeight + 2 * BORDER_WIDTH);
    QRect yawRect(-(yawRadius + BORDER_WIDTH),
                  spacing - yawRadius - BORDER_WIDTH,
                  (yawRadius + BORDER_WIDTH) * 2,
                  (yawRadius + BORDER_WIDTH) * 2);

    // 绘制动态部分
    p.translate(width() / 2, height() / 2); // 移到窗口中心
    drawRoll(&p, rollRect);
    drawPitch(&p, pitchRect);
    drawYaw(&p, yawRect);
}

void EulerWidget::resizeEvent(QResizeEvent *)
{
    updateCachedBackground();
}

void EulerWidget::updateCachedBackground()
{
    int w = width();
    int h = height();
    m_background = QPixmap(w, h);
    m_background.fill(Qt::transparent);

    QPainter p(&m_background);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(w / 2, h / 2); // 移到窗口中心

    int rollRadius = h * ROLL_RADIUS_RATIO;
    int pitchHeight = h * PITCH_HEIGHT_RATIO;
    int yawRadius = h * YAW_RADIUS_RATIO;
    int spacing = h * SPACING_RATIO;

    // 绘制滚转背景
    QRect rollRect(-(rollRadius + BORDER_WIDTH),
                   -spacing - rollRadius - BORDER_WIDTH,
                   (rollRadius + BORDER_WIDTH) * 2,
                   (rollRadius + BORDER_WIDTH) * 2);
    p.setPen(QPen(Qt::black, BORDER_WIDTH));
    QLinearGradient rollGradient(rollRect.topLeft(), rollRect.bottomRight());
    rollGradient.setColorAt(0, ROLL_BG_COLOR);
    rollGradient.setColorAt(1, ROLL_BG_COLOR.darker(150));
    p.setBrush(rollGradient);
    p.drawEllipse(rollRect);

    // 滚转刻度线（每 30 度）
    p.setPen(QPen(Qt::black, 1));
    for (int i = 0; i < 360; i += 30) {
        p.save();
        p.translate(rollRect.center());
        p.rotate(i);
        p.drawLine(rollRadius - 5, 0, rollRadius, 0);
        p.restore();
    }

    // 绘制俯仰背景
    QRect pitchRect(-40 - BORDER_WIDTH,
                    -pitchHeight / 2 - BORDER_WIDTH,
                    80 + 2 * BORDER_WIDTH,
                    pitchHeight + 2 * BORDER_WIDTH);
    p.setPen(QPen(Qt::black, BORDER_WIDTH));
    QLinearGradient pitchGradient(pitchRect.topLeft(), pitchRect.bottomRight());
    pitchGradient.setColorAt(0, PITCH_BG_COLOR);
    pitchGradient.setColorAt(1, PITCH_BG_COLOR.darker(150));
    p.setBrush(pitchGradient);
    p.drawRect(pitchRect);

    // 绘制偏航背景
    QRect yawRect(-(yawRadius + BORDER_WIDTH),
                  spacing - yawRadius - BORDER_WIDTH,
                  (yawRadius + BORDER_WIDTH) * 2,
                  (yawRadius + BORDER_WIDTH) * 2);
    p.setPen(QPen(Qt::black, BORDER_WIDTH));
    QLinearGradient yawGradient(yawRect.topLeft(), yawRect.bottomRight());
    yawGradient.setColorAt(0, YAW_BG_COLOR);
    yawGradient.setColorAt(1, YAW_BG_COLOR.darker(150));
    p.setBrush(yawGradient);
    p.drawEllipse(yawRect);

    // 偏航刻度线（每 45 度）
    p.setPen(QPen(Qt::black, 1));
    for (int i = 0; i < 360; i += 45) {
        p.save();
        p.translate(yawRect.center());
        p.rotate(i);
        p.drawLine(yawRadius - 5, 0, yawRadius, 0);
        p.restore();
    }
}

void EulerWidget::drawRoll(QPainter *p, const QRect &rect)
{
    p->save();
    p->translate(rect.center());

    // 绘制指针
    int radius = rect.width() / 2 - BORDER_WIDTH;
    p->setPen(QPen(ROLL_LINE_COLOR, 4));
    p->rotate(-m_roll);
    p->drawLine(0, 0, 0, -(radius - 10));

    // 绘制标签
    p->restore();
    p->setFont(LABEL_FONT);
    p->setPen(Qt::black);
    p->drawText(rect.left() + BORDER_WIDTH,
                rect.top() - LABEL_OFFSET,
                QString("Roll: %1°").arg(m_roll, 0, 'f', 1));
}

void EulerWidget::drawPitch(QPainter *p, const QRect &rect)
{
    p->save();
    p->translate(rect.center());

    // 绘制俯仰条
    float barLength = rect.height() * PITCH_BAR_LENGTH_RATIO / PITCH_HEIGHT_RATIO;
    float barY = -barLength * qSin(qDegreesToRadians(m_pitch));
    p->setPen(QPen(PITCH_LINE_COLOR, 4));
    p->drawLine(0, 0, 0, barY);

    // 绘制标签
    p->restore();
    p->setFont(LABEL_FONT);
    p->setPen(Qt::black);
    p->drawText(rect.left() + BORDER_WIDTH,
                rect.top() - LABEL_OFFSET,
                QString("Pitch: %1°").arg(m_pitch, 0, 'f', 1));
}

void EulerWidget::drawYaw(QPainter *p, const QRect &rect)
{
    p->save();
    p->translate(rect.center());

    // 绘制偏航箭头
    int radius = rect.width() / 2 - BORDER_WIDTH;
    float rad = qDegreesToRadians(-m_yaw);
    QPointF dir(qCos(rad) * (radius - 10), qSin(rad) * (radius - 10));
    p->setPen(QPen(YAW_LINE_COLOR, 3));
    p->drawLine(QPointF(0, 0), dir);

    // 绘制标签
    p->restore();
    p->setFont(LABEL_FONT);
    p->setPen(Qt::black);
    p->drawText(rect.left() + BORDER_WIDTH,
                rect.bottom() + LABEL_OFFSET + 10,
                QString("Yaw: %1°").arg(m_yaw, 0, 'f', 1));
}

FormPlayMPU6050::FormPlayMPU6050(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormPlayMPU6050)
    , m_model(nullptr)
{
    ui->setupUi(this);

    m_model = new QStandardItemModel(4, 3, this);
    m_model->setHorizontalHeaderLabels({"X", "Y", "Z"});
    m_model->setVerticalHeaderLabels({"Accel", "Gyro", "Euler", "Temp"});
    ui->tableView->setModel(m_model);
    m_drawWidget = new EulerWidget(this);
    ui->gLayPlane->addWidget(m_drawWidget);
}

FormPlayMPU6050::~FormPlayMPU6050()
{
    delete ui;
}

void FormPlayMPU6050::onRecvMPU(const QByteArray &data)
{
    int start = data.indexOf("<MPU>");
    int end = data.indexOf("<END>", start);

    if (start != -1 && end != -1 && end > start) {
        QByteArray packet = data.mid(start + 5, end - (start + 5));

        QList<QByteArray> parts = packet.split(',');
        if (parts.size() == 10) {
            float ax = parts[0].toFloat();
            float ay = parts[1].toFloat();
            float az = parts[2].toFloat();
            float gx = parts[3].toFloat();
            float gy = parts[4].toFloat();
            float gz = parts[5].toFloat();
            float roll = parts[6].toFloat();
            float pitch = parts[7].toFloat();
            float yaw = parts[8].toFloat();
            float temp = parts[9].toFloat();

            m_model->setItem(0, 0, new QStandardItem(QString::number(ax, 'f', 2)));
            m_model->setItem(0, 1, new QStandardItem(QString::number(ay, 'f', 2)));
            m_model->setItem(0, 2, new QStandardItem(QString::number(az, 'f', 2)));

            m_model->setItem(1, 0, new QStandardItem(QString::number(gx, 'f', 2)));
            m_model->setItem(1, 1, new QStandardItem(QString::number(gy, 'f', 2)));
            m_model->setItem(1, 2, new QStandardItem(QString::number(gz, 'f', 2)));

            m_model->setItem(2, 0, new QStandardItem(QString::number(roll, 'f', 2)));
            m_model->setItem(2, 1, new QStandardItem(QString::number(pitch, 'f', 2)));
            m_model->setItem(2, 2, new QStandardItem(QString::number(yaw, 'f', 2)));

            m_model->setItem(3, 0, new QStandardItem(QString::number(temp, 'f', 2)));
            m_model->setItem(3, 1, new QStandardItem(""));
            m_model->setItem(3, 2, new QStandardItem(""));

            m_drawWidget->setEuler(roll, pitch, yaw);
        }
    }
}
