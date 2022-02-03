#include "mainwindow.h"
#include "cobs.h"

void MainWindow::initQmlTerminal()
{
    qmlTerminalInterface = new QmlTerminalInterface();

    QQmlContext *context = ui->quickWidget->rootContext();
    context->setContextProperty("dataPlotter",qmlTerminalInterface);

    loadQmlFile(QUrl::fromLocalFile(":/qml/ExampleQmlTerminal.qml"));
}

void MainWindow::loadCompressedQml(QByteArray data)
{
    data = COBS::decode(data);
    data = qUncompress(data);

    QFile file(QString(QCoreApplication::applicationDirPath()) + QString("/terminal.qml"));
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        file.write(data);
        file.close();
    }

    loadQmlFile(QUrl::fromLocalFile(file.fileName()));
}

void MainWindow::loadQmlFile(QUrl url)
{
    ui->plainTextEditQmlLog->clear();
    ui->plainTextEditQmlLog->appendPlainText(url.toString());
    ui->quickWidget->setSource(QUrl());
    ui->quickWidget->engine()->clearComponentCache();
    ui->quickWidget->setSource(url);
}

void MainWindow::on_pushButtonQmlReload_clicked()
{
    QUrl url = ui->quickWidget->source();
    loadQmlFile(url);
}

void MainWindow::on_pushButtonQmlLoad_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load file"), "", tr("Qml file (*.qml);;Any file (*.*)"));
    if (fileName.isEmpty())
        return;

    loadQmlFile(QUrl::fromLocalFile(fileName));
}

void MainWindow::on_pushButtonQmlSaveTemplate_clicked()
{
    QString defaultName = QString("terminal.qml");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save qml terminal template"), defaultName, tr("QML file (*.qml)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QFile qmlFile(":/qml/ExampleQmlTerminal.qml");
        qmlFile.open(QFile::ReadOnly);
        QByteArray data = qmlFile.readAll();
        qmlFile.close();
        file.write(data);
        file.close();
    } else {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Cant write to file."));
        msgBox.setInformativeText(tr("This may be because file is opened in another program."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void MainWindow::on_pushButtonQmlExport_clicked()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("Export qml in compressed format."));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setButtonText(QMessageBox::Yes, tr("To clipboard"));
    msgBox.setButtonText(QMessageBox::Ok, tr("To file"));
    auto checkBox = new QCheckBox(&msgBox);
    checkBox->setText(tr("Export as C array"));
    checkBox->setChecked(true);
    msgBox.setCheckBox(checkBox);
    int returnValue = msgBox.exec();

    QFile qmlFile(ui->quickWidget->source().toLocalFile());
    qmlFile.open(QFile::ReadOnly);
    QByteArray data = qmlFile.readAll();
    qmlFile.close();
    while(data.contains("\n "))
        data.replace("\n ","\n");
    data = qCompress(data,9);
    data = COBS::encode(data);

    if(checkBox->isChecked()) {
        QByteArray newData;
        newData.append(QString("const unsigned char terminalQml[%1] = {'$','$','Q',").arg(data.length()+3).toLocal8Bit());
        for(auto it = data.begin(); it != data.end(); it++)
        {
            newData.append(QString::number((byte)*it).toLocal8Bit());
            newData.append(",");
        }
        newData.remove(newData.length()-1,1);
        newData.append("};");
        data=newData;
    }

    if (returnValue == QMessageBox::Yes) {
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(data);
    } else if (returnValue == QMessageBox::Ok) {
        QString defaultName = QString("terminal.txt");
        QString fileName = QFileDialog::getSaveFileName(this, tr("Export qml in compressed format"), defaultName, tr("Text file (*.*)"));
        if (fileName.isEmpty())
            return;
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Truncate)) {
            file.write(data);
            file.close();
        } else {
            QMessageBox msgBox(this);
            msgBox.setText(tr("Cant write to file."));
            msgBox.setInformativeText(tr("This may be because file is opened in another program."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
    }
}

void MainWindow::on_quickWidget_statusChanged(const QQuickWidget::Status &arg1)
{
    if(arg1 == QQuickWidget::Error) {
        auto errors = ui->quickWidget->errors();
        for(const auto &error : qAsConst(errors))
            ui->plainTextEditQmlLog->appendHtml("<font color=red>" + error.toString() +"<\font color>");
    } else if(arg1 == QQuickWidget::Ready) {
        ui->plainTextEditQmlLog->appendHtml("<font color=green>" + tr("File loaded") +"<\font color>");
    }
}
