#ifndef FORMPLAYMPU6050_H
#define FORMPLAYMPU6050_H

#include <QPainter>
#include <QStandardItemModel>
#include <QWidget>

class EulerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EulerWidget(QWidget *parent = nullptr);

    // 保持原有接口
    void setEuler(float roll, float pitch, float yaw);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawRoll(QPainter *p, const QRect &rect);
    void drawPitch(QPainter *p, const QRect &rect);
    void drawYaw(QPainter *p, const QRect &rect);
    void updateCachedBackground();

    // 配置参数
    static constexpr int MIN_SIZE = 300;                  // 最小尺寸
    static constexpr float ROLL_RADIUS_RATIO = 0.25;      // 滚转圆半径比例
    static constexpr float PITCH_HEIGHT_RATIO = 0.1;      // 俯仰矩形高度比例
    static constexpr float PITCH_BAR_LENGTH_RATIO = 0.15; // 俯仰条长度比例
    static constexpr float YAW_RADIUS_RATIO = 0.1;        // 偏航圆半径比例
    static constexpr float SPACING_RATIO = 0.35;          // 仪表盘间距比例
    static constexpr int LABEL_OFFSET = 15;               // 标签偏移
    static constexpr int BORDER_WIDTH = 2;                // 表盘边框宽度
    static const QColor ROLL_BG_COLOR;                    // 滚转背景渐变色
    static const QColor PITCH_BG_COLOR;                   // 俯仰背景渐变色
    static const QColor YAW_BG_COLOR;                     // 偏航背景渐变色
    static const QColor ROLL_LINE_COLOR;                  // 滚转指针颜色
    static const QColor PITCH_LINE_COLOR;                 // 俯仰条颜色
    static const QColor YAW_LINE_COLOR;                   // 偏航箭头颜色
    static const QFont LABEL_FONT;                        // 标签字体（Hello Kitty 风格）

    float m_roll = 0.0f;  // 滚转角（度）
    float m_pitch = 0.0f; // 俯仰角（度）
    float m_yaw = 0.0f;   // 偏航角（度）
    QPixmap m_background; // 缓存静态背景
};

inline const QColor EulerWidget::ROLL_BG_COLOR = QColor(255, 182, 193, 150);  // 浅粉色
inline const QColor EulerWidget::PITCH_BG_COLOR = QColor(255, 105, 180, 150); // 粉红色
inline const QColor EulerWidget::YAW_BG_COLOR = QColor(255, 192, 203, 150);   // 粉白色
inline const QColor EulerWidget::ROLL_LINE_COLOR = QColor(255, 20, 147);      // 深粉色
inline const QColor EulerWidget::PITCH_LINE_COLOR = QColor(199, 21, 133);     // 紫红色
inline const QColor EulerWidget::YAW_LINE_COLOR = QColor(219, 112, 147);      // 淡粉色
inline const QFont EulerWidget::LABEL_FONT = QFont("Comic Sans MS", 10, QFont::Bold);

namespace Ui {
class FormPlayMPU6050;
}

class FormPlayMPU6050 : public QWidget
{
    Q_OBJECT

public:
    explicit FormPlayMPU6050(QWidget *parent = nullptr);
    ~FormPlayMPU6050();
public slots:
    void onRecvMPU(const QByteArray &data);

private:
    Ui::FormPlayMPU6050 *ui;
    QStandardItemModel *m_model;
    EulerWidget *m_drawWidget;
};

#endif // FORMPLAYMPU6050_H
