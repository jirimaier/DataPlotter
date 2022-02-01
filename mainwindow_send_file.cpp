#include "mainwindow.h"

void MainWindow::fileRequest(QByteArray message, MessageTarget::enumMessageTarget source)
{
    QByteArrayList list = message.split(',');

    if(!list.isEmpty() && list.first().toLower()=="new")
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Send file"), "", "Any file (*.*)");
        if (fileName.isEmpty())
          return;
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
          QByteArray text = file.readAll();
          fileSender.newFile(text);
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Cant open file."));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }

        list.removeFirst();

        if(list.isEmpty()) return;
    }

    if(list.length()==0)
    {
        QByteArray result = fileSender.nextPart("","");
        if(!result.isEmpty()) printMessage(tr("Invalid file request").toUtf8(),result , MessageLevel::error, source);
    }
    else if(list.length()==1)
    {
        QByteArray result = fileSender.nextPart(list.first(),"");
        if(!result.isEmpty()) printMessage(tr("Invalid file request").toUtf8(),result , MessageLevel::error, source);
    }
    else if(list.length()==2)
    {
        QByteArray result = fileSender.nextPart(list.first(),list.last());
        if(!result.isEmpty()) printMessage(tr("Invalid file request").toUtf8(),result , MessageLevel::error, source);
    }
    else
        printMessage(tr("Invalid file request").toUtf8(),tr("Too many arguments in header").toUtf8() , MessageLevel::error, source);
}
