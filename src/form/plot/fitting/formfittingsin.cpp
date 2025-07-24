#include "formfittingsin.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QMouseEvent>
#include <QtMath>
#include "ImageViewer/imageviewer.h"
#include "datadef.h"
#include "funcdef.h"
#include "httpclient.h"
#include "ui_formfittingsin.h"

ImageViewer *imageViewer = nullptr;
QString m_urlCalculate;
FormFittingSin::FormFittingSin(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormFittingSin)
{
    ui->setupUi(this);
    init();

    ui->labelPlotSin->installEventFilter(this);
}

FormFittingSin::~FormFittingSin()
{
    delete ui;
}

void FormFittingSin::init()
{
    ui->labelFormula->setText("y = y<sub>0</sub> + A &middot; sin(w &pi; (x - x<sub>c</sub>))");
    ui->labelFormula->setTextFormat(Qt::RichText);
    ui->labelPlotSin->setToolTip(tr("Double click to zoom."));
    ui->labelPlotSin->setScaledContents(true);

    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(3);
    m_model->setHeaderData(0, Qt::Horizontal, "index");
    m_model->setHeaderData(1, Qt::Horizontal, "fitting curve V(14bit)");
    m_model->setHeaderData(2, Qt::Horizontal, "fitting curve Raw(14bit)");
    ui->tableViewFittingCurveData->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->tableViewFittingCurveData->setModel(m_model);

    m_urlCalculate = SETTING_CONFIG_GET(CFG_GROUP_SETTING,
                                        CFG_SETTING_CALCULATE_URL,
                                        URL_FITTING_SIN);
}

void FormFittingSin::fillFittingCurveData()
{
    m_model->removeRows(0, m_model->rowCount());

    const int maxX = 1310 * 2;

    for (int x = 0; x < maxX; ++x) {
        double y = m_sin.y0 + m_sin.A * qSin(M_PI * (x - m_sin.xc) / m_sin.w);

        qint32 signedRaw = qRound(y / 3.3 * (1 << 13));

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(QString::number(x));
        rowItems << new QStandardItem(QString::number(y, 'f', 6));
        rowItems << new QStandardItem(QString::number(signedRaw));

        m_model->appendRow(rowItems);
    }
}

bool FormFittingSin::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->labelPlotSin && event->type() == QEvent::MouseButtonDblClick) {
        if (!m_pixmap.isNull()) {
            if (!imageViewer) {
                imageViewer = new ImageViewer(this);
                imageViewer->setAttribute(Qt::WA_DeleteOnClose);
                imageViewer = nullptr;
            }
            ImageViewer *viewer = new ImageViewer(this);
            viewer->setAttribute(Qt::WA_DeleteOnClose);
            viewer->setImage(m_pixmap);
            viewer->show();
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void FormFittingSin::doCorrection(const QVector<double> &v14)
{
    ui->textBrowserSinLog->append("===== start correction =====");

    if (v14.empty()) {
        ui->textBrowserSinLog->append("v14 is Empty!");
        return;
    }

    int closest_index = 0;
    double min_abs_val = std::abs(v14[0]);
    for (int i = 1; i < v14.size(); ++i) {
        double abs_val = std::abs(v14[i]);
        if (abs_val < min_abs_val) {
            min_abs_val = abs_val;
            closest_index = i;
        }
    }

    int x0 = 1310;

    QJsonArray x_arr;
    QJsonArray y_arr;
    for (int i = 0; i < v14.size(); ++i) {
        int x = x0 + (i - closest_index);
        x_arr.append(x);
        y_arr.append(v14[i]);
    }

    QJsonObject obj;
    obj[KEY_X] = x_arr;
    obj[KEY_Y] = y_arr;

    QUrl url(m_urlCalculate);
    HttpClient *client = new HttpClient(this);
    connect(client, &HttpClient::success, this, [=](const QJsonDocument &resp) {
        QJsonObject result = resp.object();

        m_sin.A = result[KEY_A].toDouble();
        m_sin.w = result[KEY_w].toDouble();
        m_sin.xc = result[KEY_xc].toDouble();
        m_sin.y0 = result[KEY_y0].toDouble();
        QString imageUrl = result[KEY_IMG_URL].toString();

        ui->lineEdit_A->setText(QString::number(m_sin.A));
        ui->lineEdit_w->setText(QString::number(m_sin.w));
        ui->lineEdit_xc->setText(QString::number(m_sin.xc));
        ui->lineEdit_y0->setText(QString::number(m_sin.y0));
        ui->labelFormula->setText(QString("y = %1 + %2 &middot; sin(%3 &pi; (x - %4))")
                                      .arg(m_sin.y0)
                                      .arg(m_sin.A)
                                      .arg(m_sin.w)
                                      .arg(m_sin.xc));
        QString msg = QString("response: %1")
                          .arg(QString(QJsonDocument(resp).toJson(QJsonDocument::Compact)));
        ui->textBrowserSinLog->append(msg);
        LOG_INFO(msg);
        ui->labelFormula->setTextFormat(Qt::RichText);
        fillFittingCurveData();
        client->getImage(QUrl(imageUrl));
    });

    connect(client, &HttpClient::imageLoaded, this, [=](const QPixmap &pixmap) {
        QString msg = QString("Fitting Sin recv img size: (%1, %2)")
                          .arg(pixmap.size().width())
                          .arg(pixmap.size().height());
        ui->textBrowserSinLog->append(msg);
        LOG_INFO(msg);

        if (pixmap.isNull()) {
            QString msg = "pic is empty!";
            ui->textBrowserSinLog->append(msg);
            LOG_WARN(msg);
            return;
        }
        m_pixmap = pixmap;
        ui->labelPlotSin->setPixmap(m_pixmap);
    });

    connect(client, &HttpClient::imageFailed, this, [=](const QString &err) {
        QString msg = "pic load failed: " + err;
        ui->textBrowserSinLog->append(msg);
        LOG_WARN(msg);
    });
    connect(client, &HttpClient::failure, this, [&](const QString &err) {
        QString msg = "fitting failed: " + err;
        ui->textBrowserSinLog->append(msg);
        LOG_WARN(msg);
    });

    client->post(url, obj);
}
