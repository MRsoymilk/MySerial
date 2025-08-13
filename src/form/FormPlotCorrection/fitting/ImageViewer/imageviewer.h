#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QDialog>
#include <QPixmap>

class QLabel;
class QScrollArea;

class ImageViewer : public QDialog
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1.0;
};
#endif // IMAGEVIEWER_H
