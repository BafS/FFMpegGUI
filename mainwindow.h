#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include <QJsonDocument>
#include <QJsonObject>

#include <QStandardItemModel>
#include <QStandardItem>

#include <QTime>


#include <QVideoWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QGraphicsVideoItem>
#include <QVideoProbe>
#include <QVideoSurfaceFormat>
#include <QGraphicsVideoItem>

#include <QDateTime>

#include <QThread>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_quitButton_clicked();

    void on_openFileButton_clicked();


    //void executeCommand();
    void outputCommand();

    void on_execute_clicked();

    void on_actionQuit_triggered();

    void ffprobeInfo();

    void on_filePathOut_textChanged(const QString &arg1);

    void on_timeStart_timeChanged(const QTime &time);

    void loadPreview();

    void on_filePath_textChanged(const QString &arg1);

    void on_play_clicked();

    void on_pause_clicked();

    void on_saveFileBtn_clicked();

private:
    Ui::MainWindow *ui;

    QProcess commandProcess;

    QString ffprobeBin;

    QString ffmpegBin;

    qint64 QTimeToMS(QTime &);

    QMediaPlayer* player;

    QVideoWidget* videoWidget;
};

#endif // MAINWINDOW_H
