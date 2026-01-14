#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class FormEasy;
class FormExpert;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void menuLanguageSelect(QAction *selectedAction);
    void menuThemeSelect(QAction *selectedTheme);
    void menuAlgorithmSelect(QAction *selectedAlgorithm);
    void on_actionEasy_triggered();
    void on_actionExpert_triggered();

private:
    void init();
    void initMsgBar();
    void initLanguage();
    void initTheme();
    void initAlgorithm();
    void setLanguage(const QString &language);
    void setTheme(const QString &theme);
    void initStackWidget();
    void initMode();
    void menuModeSelect(QAction *selectedMode);
    void setAlgorithm(const QString &algorithm);
    void setMode(const QString &mode);

private:
    Ui::MainWindow *ui;

private:
    QPixmap m_background;
    FormEasy *m_formEasy;
    FormExpert *m_formExpert;

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
