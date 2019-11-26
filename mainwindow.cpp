#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL( timeout()), this, SLOT(updateTimerTimeout()));
    updateTimer->start(15);
    timerRunning = false;
    cumulativeStoppedTime = 0;
    pauseTime = QDateTime();
    runningTime = QTime::fromMSecsSinceStartOfDay(0);
    ui->timerDurtationField->setStyleSheet("background: #000000; color: #666666; border: 0px ;");
    ui->elapsedTimeField->setStyleSheet("background: #000000; color: #440000; border: 0px ;");
    ui->remainingTimeField->setStyleSheet("background: #000000; color: #00aa00; border: 0px ;");
    ui->remainingTimeField->setText("");
    ui->timerDurtationField->setText("");
    ui->progressBar->setValue(0);


    remoteCurrentTime = QString("");
    remoteTimeRemaining = QString("");

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(5050, QAbstractSocket::ShareAddress);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    qDebug() << "bound to port 5050";
    this->reset();
}

void MainWindow::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QByteArray data = datagram.data();
        QList<QByteArray> rawArgs = data.split(NULL);
        QList<QByteArray> args;

        QList<QByteArray>::iterator i;
        for (i = rawArgs.begin(); i != rawArgs.end(); ++i)
            if( !(*i).isEmpty() )
                args << (*i);

        QString str = QString::fromUtf8(args[0]).toLower();
        QList<QString> path = str.split("/");
        path.removeFirst();




//        qDebug() << "Datagram data: " << datagram.data();
//        qDebug() << "Arguments: " << args;
//        qDebug() << "String: " << str;
        qDebug() << "Path: " << path;

        if( path.size() > 0 ) {
            if( path[0] == "timer" ) {
                if( path.size() > 1 ) {
                    if( path[1] == "start" ) {
                        this->start();
                    } else if (path[1] == "stop" ) {
                        this->stop();
                    } else if ( path[1] == "reset" ) {
                        this->reset();
                    } else if ( path[1] == "add") {
                        if( path.size() > 2 ) {
                            int mod = path[2].toInt();
                            qDebug() << "add " << mod << " to the timer";
                            runningTime = runningTime.addSecs(mod);
                        }
                    } else if( path[1] == "remove" ) {
                        if( path.size() > 2 ) {
                            int mod = path[2].toInt() * -1;
                            qDebug() << "remove " << mod << " to the timer";
                            runningTime = runningTime.addSecs(mod);
                        }
                    }
                }
            } else if( path[0] == "ping") {
                QString reply = "/pong";
                if( path.size() > 1 ) {
                    reply = reply + "/" + path[1];
                }
                int size = reply.size() + (4 - (reply.length() % 4));

                QByteArray y = reply.toUtf8().leftJustified(size, 0);   // null padding to next 4 bytes"
                udpSocket->writeDatagram(y ,datagram.senderAddress(), 5050);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start()
{
    qDebug() << "MainWindow::start() called";
    if( !timerRunning )
    {
        timerRunning = true;
        startTime = QDateTime::currentDateTime();
        if( !pauseTime.isNull() )
        {
            cumulativeStoppedTime = cumulativeStoppedTime + pauseTime.msecsTo( QDateTime::currentDateTime() );
        }
        pauseTime = QDateTime();
        ui->elapsedTimeField->setStyleSheet("background: #000000; color: #ff0000; border: 0px ;");
    }
}

void MainWindow::on_startButton_clicked()
{
    qDebug() << "MainWindow::on_startButton_clicked() called";
    this->start();
}

void MainWindow::stop()
{
    qDebug() << "MainWindow::stop() called";
    timerRunning = false;
    cumulativeTime = cumulativeTime + startTime.msecsTo(QDateTime::currentDateTime());
}

void MainWindow::on_stopButton_clicked()
{
    qDebug() << "MainWindow::on_stopButton_clicked() called";
    this->stop();
}

void MainWindow::reset()
{
    qDebug() << "MainWindow::reset() called";
    startTime = QDateTime::currentDateTime();
    cumulativeTime = 0;
    ui->elapsedTimeField->setText("00:00:00.00");
    runningTime = QTime::fromMSecsSinceStartOfDay(0);
    ui->remainingTimeField->setText("");
    ui->timerDurtationField->setText("");

    if( !timerRunning )
    {
        ui->elapsedTimeField->setStyleSheet("background: #000000; color: #440000; border: 0px ;");
    }
}

void MainWindow::on_resetButton_clicked()
{
    qDebug() << "MainWindow::on_resetButton_clicked() called";
    this->reset();
}

void MainWindow::updateTimerTimeout()
{
    qint64 msecs = startTime.msecsTo(QDateTime::currentDateTime());
//    qDebug() << "current time " << msecs << " but stoppage time is " << cumulativeTime;
    if( timerRunning )
    {
        QTime displayTime = QTime::fromMSecsSinceStartOfDay(msecs + cumulativeTime);
        QString time = displayTime.toString("hh:mm:ss.zzz");
        time.chop(1);
        ui->elapsedTimeField->setText(time);

        time.chop(3);
        if( remoteCurrentTime != time )
        {
            QString reply = "/timer/updateTimer/" + time;
            int size = reply.size() + (4 - (reply.length() % 4));

            QByteArray y = reply.toUtf8().leftJustified(size, 0);   // null padding to next 4 bytes"
            udpSocket->writeDatagram(y ,QHostAddress("255.255.255.255"), 5050);
            remoteCurrentTime = time;
        }

        if( runningTime.secsTo(QTime::fromMSecsSinceStartOfDay(0)) == 0 )
        {
            ui->remainingTimeField->setText("");
        } else {
            QTime remaining = QTime::fromMSecsSinceStartOfDay(displayTime.msecsTo(runningTime));
            if( remaining.isValid() )
            {
                QString time2 = remaining.toString("hh:mm:ss.zzz");
                time2.chop(1);
                ui->remainingTimeField->setText(time2);

                time2.chop(3);
                if( remoteTimeRemaining != time2 )
                {
                    QString reply = "/timer/updateTimeRemaining/" + time;
                    int size = reply.size() + (4 - (reply.length() % 4));

                    QByteArray y = reply.toUtf8().leftJustified(size, 0);   // null padding to next 4 bytes"
                    udpSocket->writeDatagram(y ,QHostAddress("255.255.255.255"), 5050);
                    remoteTimeRemaining = time2;
                }

                float percentage = ((float)remaining.msecsSinceStartOfDay() / (float)runningTime.msecsSinceStartOfDay()) * 100;
                qDebug() << "Percentage complete is: " << remaining.msecsSinceStartOfDay() << "/" << runningTime.msecsSinceStartOfDay() << " which is '" << percentage << "'";
                ui->progressBar->setValue(percentage);


            } else {
                ui->remainingTimeField->setText("00:00:00.00");
            }
        }

    }

    if( runningTime.secsTo(QTime::fromMSecsSinceStartOfDay(0)) == 0 )
    {
        ui->timerDurtationField->setText("");
    } else {
        QString time3 = runningTime.toString("hh:mm:ss.zzz");
        time3.chop(1);
        ui->timerDurtationField->setText(time3);
    }
}

void MainWindow::on_sec1down_clicked()
{
    runningTime = runningTime.addSecs(-1);
}

void MainWindow::on_hr1down_clicked()
{
    runningTime = runningTime.addSecs(-3600);
}

void MainWindow::on_hr1up_clicked()
{
    runningTime = runningTime.addSecs(3600);
}

void MainWindow::on_min10down_clicked()
{
    runningTime = runningTime.addSecs(-60*10);
}

void MainWindow::on_min10up_clicked()
{
    runningTime = runningTime.addSecs(60*10);
}

void MainWindow::on_min1down_clicked()
{
    runningTime = runningTime.addSecs(-60);
}

void MainWindow::on_min1up_clicked()
{
    runningTime = runningTime.addSecs(60);
}

void MainWindow::on_sec10down_clicked()
{
    runningTime = runningTime.addSecs(-10);
}

void MainWindow::on_sec10up_clicked()
{
    runningTime = runningTime.addSecs(10);
}

void MainWindow::on_sec1up_clicked()
{
    runningTime = runningTime.addSecs(1);
}
