#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QStyle>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Мій Годинник");
    resize(400, 400);

    // --- 1. Створення кнопки "Тема" ---
    themeButton = new QPushButton("Тема", this);
    themeButton->setGeometry(10, 10, 80, 30);
    themeButton->setCursor(Qt::PointingHandCursor); // Курсор-рука при наведенні

    // --- 2. Створення меню ---
    QMenu *themeMenu = new QMenu(themeButton);
    themeMenu->addAction("Світла", this, [=](){ applyTheme("light"); });
    themeMenu->addAction("Темна", this, [=](){ applyTheme("dark"); });
    themeMenu->addAction("Ретро", this, [=](){ applyTheme("retro"); });
    themeMenu->addAction("Футуристична", this, [=](){ applyTheme("future"); });

    themeButton->setMenu(themeMenu);

    // --- 3. Таймер ---
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timer->start(1000);

    // --- 4. Трей ---
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction("Розгорнути", this, &MainWindow::showNormal);
    trayMenu->addAction("Вихід", qApp, &QCoreApplication::quit);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
        if(reason == QSystemTrayIcon::Trigger) this->showNormal();
    });

    // --- 5. Застосовуємо тему ---
    applyTheme("light");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- ФУНКЦІЯ НАЛАШТУВАННЯ ТЕМ ---
void MainWindow::applyTheme(QString themeName)
{
    if (themeName == "light") {
        backgroundColor = Qt::white;
        hourColor = Qt::black;
        minuteColor = Qt::darkGray;
        secondColor = Qt::red;
        textColor = Qt::black;
    }
    else if (themeName == "dark") {
        backgroundColor = QColor(0, 0, 50);
        hourColor = Qt::white;
        minuteColor = Qt::lightGray;
        secondColor = Qt::yellow;
        textColor = Qt::white;
    }
    else if (themeName == "retro") {
        backgroundColor = QColor(139, 69, 19);
        hourColor = QColor(50, 20, 0);
        minuteColor = QColor(80, 40, 0);
        secondColor = QColor(255, 222, 173);
        textColor = QColor(255, 248, 220);
    }
    else if (themeName == "future") {
        backgroundColor = Qt::black;
        hourColor = Qt::cyan;
        minuteColor = QColor(0, 200, 200);
        secondColor = Qt::magenta;
        textColor = Qt::cyan;
    }

    // --- СТИЛІЗАЦІЯ КНОПКИ ТЕМ ---

    QString txtColorName = textColor.name();

    QString menuBgColor = QString("rgba(%1, %2, %3, 200)")
                              .arg(backgroundColor.red())
                              .arg(backgroundColor.green())
                              .arg(backgroundColor.blue());

    // 1. Стиль для КНОПКИ
    QString buttonStyle = QString(
                              "QPushButton {"
                              "  background-color: transparent;"
                              "  color: %1;"
                              "  border: 1px solid %1;"
                              "  border-radius: 5px;"
                              "}"
                              "QPushButton::menu-indicator { image: none; }"
                              "QPushButton:hover {"
                              "  background-color: rgba(128, 128, 128, 50);"
                              "}"
                              ).arg(txtColorName);

    themeButton->setStyleSheet(buttonStyle);

    // 2. Стиль для СПАДНОГО ВІКНА
    QString menuStyle = QString(
                            "QMenu {"
                            "  background-color: %2;"
                            "  color: %1;"
                            "  border: 1px solid %1;"
                            "}"
                            "QMenu::item:selected {"
                            "  background-color: rgba(255, 255, 255, 50);"
                            "}"
                            ).arg(txtColorName).arg(menuBgColor);

    themeButton->menu()->setStyleSheet(menuStyle);

    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QTime time = QTime::currentTime();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. Заливаємо фон
    painter.fillRect(event->rect(), backgroundColor);

    int w = width();
    int h = height();
    int clockSize = qMin(w, (int)(h * 0.75));

    // --- АНАЛОГОВИЙ ГОДИННИК ---
    painter.save();

    // Центр малювання: 50% ширини, 40% висоти
    painter.translate(w / 2, h * 0.4);

    qreal scaleFactor = clockSize / 220.0;
    painter.scale(scaleFactor, scaleFactor);

    // Риски
    painter.setPen(textColor);
    for (int i = 0; i < 60; ++i) {
        if (i % 5 == 0) painter.drawLine(0, -88, 0, -96);
        else painter.drawLine(0, -92, 0, -96);
        painter.rotate(6.0);
    }

    // Стрілки
    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    drawHand(&painter, hourColor, 50, 0, 4);
    painter.restore();

    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    drawHand(&painter, minuteColor, 70, 0, 2);
    painter.restore();

    painter.save();
    painter.rotate(6.0 * time.second());
    drawHand(&painter, secondColor, 80, 0, 1);
    painter.restore();

    painter.restore(); // Відновлюємо координати


    // --- ЦИФРОВИЙ ГОДИННИК ---

    QFont font = painter.font();
    // Шрифт залежить від висоти вікна
    font.setPixelSize(qMax(16, h / 15));
    if (backgroundColor == Qt::black) font.setBold(true);
    painter.setFont(font);

    // Визначаємо зону для тексту
    int rectWidth = w * 0.6;
    int rectHeight = h * 0.15;
    int rectX = (w - rectWidth) / 2;
    int rectY = h * 0.8;

    QRect digitalRect(rectX, rectY, rectWidth, rectHeight);

    // Встановлюємо колір тексту
    painter.setPen(textColor);

    // Малюємо ТІЛЬКИ текст (рядок з малюванням рамки видалено)
    painter.drawText(digitalRect, Qt::AlignCenter, time.toString("hh:mm:ss"));
}

void MainWindow::drawHand(QPainter *painter, const QColor &color, int length, double, int width)
{
    painter->setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(0, 0, 0, -length);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (this->isVisible()) {
        event->ignore();
        this->hide();
        trayIcon->showMessage("Годинник", "Програма згорнута в трей");
    }
}
