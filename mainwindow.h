#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkDatagram>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void on_resetButton_clicked();

    void updateTimerTimeout();

    void on_sec1down_clicked();

    void on_hr1down_clicked();

    void on_hr1up_clicked();

    void on_min10down_clicked();

    void on_min10up_clicked();

    void on_min1down_clicked();

    void on_min1up_clicked();

    void on_sec10down_clicked();

    void on_sec10up_clicked();

    void on_sec1up_clicked();

    void readPendingDatagrams();

private:
    Ui::MainWindow *ui;
    QTimer *updateTimer;
    QDateTime startTime;
    QDateTime pauseTime;
    QTime runningTime;
    qint64 cumulativeStoppedTime;
    qint64 cumulativeTime;
    bool timerRunning;
    QUdpSocket *udpSocket;

    QString remoteCurrentTime;
    QString remoteTimeRemaining;

    void start();
    void stop();
    void reset();

};

#endif // MAINWINDOW_H
