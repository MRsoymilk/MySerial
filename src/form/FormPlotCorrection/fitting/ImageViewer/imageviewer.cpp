#include "imageviewer.h"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWheelEvent>

ImageViewer::ImageViewer(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("图片查看器");
    resize(800, 600);

    imageLabel = new QLabel(this);
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true); // QLabel 自己缩放内容

    scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);
    setLayout(layout);
}

void ImageViewer::setImage(const QPixmap &pixmap)
{
    imageLabel->setPixmap(pixmap);
    scaleFactor = 1.0;
    imageLabel->adjustSize();
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (imageLabel->pixmap().isNull())
        return;

    const double scaleStep = 1.15;
    if (event->angleDelta().y() > 0) {
        scaleFactor *= scaleStep;
    } else {
        scaleFactor /= scaleStep;
    }
    scaleFactor = qBound(0.1, scaleFactor, 10.0); // 限制缩放范围

    imageLabel->resize(scaleFactor * imageLabel->pixmap().size());
    event->accept();
}
