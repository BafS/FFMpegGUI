#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    ffprobeBin("/usr/local/bin/ffprobe"),
    ffmpegBin("/usr/local/bin/ffmpeg")
    {

    // Load an application style
    QFile styleFile(":/qss/main.css");
    styleFile.open(QFile::ReadOnly);

    // Apply the loaded stylesheet
    QString style(styleFile.readAll());
    this->setStyleSheet(style);

    connect (&commandProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(outputCommand()));
    connect (&commandProcess, SIGNAL(readyReadStandardError()), this, SLOT(outputCommand()));

    ui->setupUi(this);

    videoWidget = new QVideoWidget(ui->graphicsView);
    player = new QMediaPlayer(this);
    //player->setNotifyInterval(1000);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_quitButton_clicked() {
    qApp->quit();
}

void MainWindow::on_openFileButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
            tr("Media Files (*.avi *.mp4 *.mp3 *.divx *.mov *.flac *.wmv *.ogg *.mkv)"));

    if (!fileName.isEmpty()) {

        QFile file(fileName);

        if (file.exists()) {

            ui->filePath->setText(fileName);

            QFileInfo finfo(fileName);
            QDir dir = finfo.absoluteDir();

            // If output field is empty, with replace the name of the video with "out"
            QString outputPath(dir.absolutePath().append("/").append(finfo.baseName()).append(".out.").append(finfo.suffix()));
            ui->filePathOut->setText(outputPath);
        }
    }
}

void MainWindow::ffprobeInfo() {

    QProcess builder;
    builder.setProcessChannelMode(QProcess::MergedChannels);

    QStringList args;

    args << "-v"; args << "quiet";
    args << "-print_format"; args << "json";
    args << "-show_format";
    args << ui->filePath->text();

    builder.start(ffprobeBin, args);

    if (!builder.waitForFinished()) {
        qDebug() << "failed:" << builder.errorString();
    }
    else {
        QString jsonStr(builder.readAll());

        QJsonDocument doc(QJsonDocument::fromJson(jsonStr.toUtf8()));
        QJsonObject json = doc.object();

        QJsonObject format(json["format"].toObject()); // We just want the "format" field

        QStandardItemModel *model = new QStandardItemModel(0, 2, this);
        model->setHorizontalHeaderItem(0, new QStandardItem(QString("Info")));
        model->setHorizontalHeaderItem(1, new QStandardItem(QString("Value")));

        ui->infoTable->setModel(model);
        ui->infoTable->verticalHeader()->setVisible(false); // remove index number

        QStandardItem *firstRow = new QStandardItem(QString("ColumnValue"));
        model->setItem(0, 0, firstRow);

        // Populate the table
        int i = 0;
        foreach (auto key, format.keys()) {
            QJsonValue val(format[key]);
            if(val.isString() || val.isDouble()) {
                model->setItem(i, 0, new QStandardItem(key));
                model->setItem(i, 1, new QStandardItem(val.toString()));
                ++i;
            }
        }

        for (int c = 0; c < ui->infoTable->horizontalHeader()->count(); ++c) {
            ui->infoTable->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
        }

        int duration(format["duration"].toString().split(".")[0].toInt());

        // Convert seconds duration to hh:mm:ss
        int seconds = (int) (duration % 60);
        duration /= 60;
        int minutes = (int) (duration % 60);
        duration /= 60; int hours = (int) (duration % 24);

        // Duration

        QTime time(hours, minutes, seconds);

        // Set timeStart and timeStop limits
        ui->timeStop->setTime(time);
        ui->timeStop->setMaximumTime(time);
        ui->timeStart->setMaximumTime(time);
        // Put the cursor to the "second" section
        ui->timeStop->setCurrentSection(QDateTimeEdit::SecondSection);
        ui->timeStart->setCurrentSection(QDateTimeEdit::SecondSection);


    }

}

void MainWindow::outputCommand() {
    QByteArray cmdoutput = commandProcess.readAllStandardOutput();
    QString txtoutput = cmdoutput;
    ui->infopanel->setTextColor(QColor("#333"));
    ui->infopanel->append(txtoutput);

    cmdoutput = commandProcess.readAllStandardError();
    txtoutput = cmdoutput;
    QColor tc = ui->infopanel->textColor(); // save
    ui->infopanel->setTextColor(QColor("#533"));
    ui->infopanel->append(txtoutput);
    ui->infopanel->setTextColor(tc); // restore
}

void MainWindow::on_execute_clicked() {
    QStringList args;

    args << "-i"; args << ui->filePath->text();
    args << "-ss"; args << ui->timeStart->text();
    args << "-to"; args << ui->timeStop->text();

    if(!ui->cutomOptions->text().isEmpty()) {
      args << ui->cutomOptions->text();
    }

    args << ui->filePathOut->text();

    qDebug() << args;

    commandProcess.start(ffmpegBin, args);
}

void MainWindow::on_actionQuit_triggered() {
    qApp->quit();
}

void MainWindow::on_filePathOut_textChanged(const QString &fileName) {
    QFile file(fileName);

    if (file.exists()) {
        ui->filePathOut->setStyleSheet("color: red");
        ui->execute->setDisabled(true);
        QColor tc = ui->infopanel->textColor(); // save
        ui->infopanel->setTextColor(QColor("red"));
        ui->feedback->setText("Output file already exists !");
        ui->infopanel->setTextColor(tc);
    } else {
        ui->filePathOut->setStyleSheet("color: inherited");
        ui->execute->setDisabled(false);
        ui->feedback->setText("");
    }
}

// time stop must be >= time start
void MainWindow::on_timeStart_timeChanged(const QTime &time) {
    ui->timeStop->setMinimumTime(time);

    QTime timeStart(ui->timeStart->time());
    qint64 ms(QTimeToMS(timeStart));

    qDebug() << ms;

    player->play();
    player->setPosition(ms);
    player->pause();
}

void MainWindow::loadPreview() {

    QMediaPlaylist* playlist = new QMediaPlaylist;
    playlist->addMedia(QUrl::fromLocalFile(ui->filePath->text()));
    playlist->setCurrentIndex(1);

    player->play();
    player->isSeekable();
    player->setMuted(true);
    player->setPlaylist(playlist);
    player->setVideoOutput(videoWidget);
    player->pause();
    videoWidget->show();
    videoWidget->hide();

    videoWidget->resize(ui->graphicsView->width(), ui->graphicsView->height());
    videoWidget->setGeometry(0, 0, ui->graphicsView->width(), ui->graphicsView->height());
    videoWidget->show();
}

void MainWindow::on_filePath_textChanged(const QString &fileName) {
    QFile file(fileName);

    bool fileExists = file.exists();

    if (!fileExists) {
        ui->filePath->setStyleSheet("color: red");
        QColor tc = ui->infopanel->textColor(); // save
        ui->infopanel->setTextColor(QColor("red"));
        ui->feedback->setText("File input does not exists !");
        ui->infopanel->setTextColor(tc);

    } else {
        ui->filePath->setStyleSheet("color: inherited");
        ui->feedback->setText("");

        ffprobeInfo();
        loadPreview();
    }

    ui->timeStart->setDisabled(!fileExists);
    ui->timeStop->setDisabled(!fileExists);
    ui->execute->setDisabled(!fileExists);
    ui->play->setDisabled(!fileExists);
    ui->pause->setDisabled(!fileExists);
}

qint64 MainWindow::QTimeToMS(QTime &time) {
    return time.hour() * 60 * 60 * 1000 + time.minute() * 60 * 1000 + time.second() * 1000;
}

void MainWindow::on_play_clicked() {
    player->play();
}


void MainWindow::on_pause_clicked() {
    player->pause();
}

void MainWindow::on_saveFileBtn_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QString());

    if (!fileName.isEmpty()) {

        QFile file(fileName);

        if (!file.exists()) {
            ui->filePathOut->setText(fileName);
        }
    }
}
