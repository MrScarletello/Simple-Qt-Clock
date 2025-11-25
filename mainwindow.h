#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Слот для перемикання теми (приймає назву теми)
    void applyTheme(QString themeName);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QSystemTrayIcon *trayIcon;
    QPushButton *themeButton;

    // Змінні кольорів
    QColor backgroundColor;
    QColor hourColor;
    QColor minuteColor;
    QColor secondColor;
    QColor textColor;

    void drawHand(QPainter *painter, const QColor &color, int length, double angle, int width);
};
#endif // MAINWINDOW_H
