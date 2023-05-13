#include "manualinputdialog.h"
#include "qcombobox.h"
#include "qdebug.h"
#include "qscrollbar.h"
#include "ui_manualinputdialog.h"

ManualInputDialog::ManualInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManualInputDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    iconRun = QIcon(":/images/icons/run.png");
    iconPause = QIcon(":/images/icons/pause.png");
    ui->pushButtonRolling->setIcon(iconPause);
    connect(&rollingTimer,&QTimer::timeout,this,&ManualInputDialog::rollingDataTimerRoutine);

    rollingEngine.globalObject().setProperty("t",0);

    ui->tableWidgetRollingSetup->setRowCount(1);
    QHeaderView *headerView = ui->tableWidgetRollingSetup->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    headerView->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    QHBoxLayout *tableButtons = new QHBoxLayout();
    tableButtons->setContentsMargins(0,0,0,0);
    QPushButton *addRowButton = new QPushButton(QIcon(":/images/icons/plus.png"),"",ui->tableWidgetRollingSetup);
    QPushButton *removeRowButton = new QPushButton(QIcon(":/images/icons/minus.png"),"",ui->tableWidgetRollingSetup);
    tableButtons->addWidget(addRowButton);
    tableButtons->addWidget(removeRowButton);
    ui->tableWidgetRollingSetup->setCellWidget(0,0,new QWidget());
    QWidget *cellWidget = ui->tableWidgetRollingSetup->cellWidget(0, 0);
    cellWidget->setLayout(tableButtons);
    //ui->tableWidgetRollingSetup->item(0,0)->setFlags(ui->tableWidgetRollingSetup->item(0,0)->flags() &  ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
    connect(addRowButton, &QPushButton::clicked, [this]() {
        this->setRollingExprRows(ui->tableWidgetRollingSetup->rowCount()+1);
    });
    connect(removeRowButton, &QPushButton::clicked, [this]() {
        if(ui->tableWidgetRollingSetup->rowCount()>2)
            this->setRollingExprRows(ui->tableWidgetRollingSetup->rowCount()-1);
    });

    setRollingExprRows(3);
    ui->tableWidgetRollingSetup->setItem(0,0, new QTableWidgetItem("5*sin(2*Pi*t)"));
    ui->tableWidgetRollingSetup->setItem(1,0, new QTableWidgetItem("2*cos(2*Pi*t) + 0.1*random()*sin(100*t)"));
}

ManualInputDialog::~ManualInputDialog()
{
    delete ui;
}

Ui::ManualInputDialog *ManualInputDialog::getUi() const
{
    return ui;
}

void ManualInputDialog::on_doubleSpinBoxRollingFreq_valueChanged(double arg1)
{
    rollingTimer.setInterval(1000/arg1);
}

void ManualInputDialog::on_pushButtonRolling_clicked()
{
    if(rollingTimer.isActive())
        rollingTimer.stop();
    else
        rollingTimer.start(1000/ui->doubleSpinBoxRollingFreq->value());
    ui->pushButtonRolling->setIcon(rollingTimer.isActive()?iconRun:iconPause);
}

void ManualInputDialog::rollingDataTimerRoutine()
{
    rollingTimestamp+=static_cast<double>(rollingTimer.interval())/1000.0;
    QString data = "$$P" + QString::number(rollingTimestamp,'g',10);
    rollingEngine.globalObject().setProperty("t",rollingTimestamp);
    for(auto &ch : rollingChannelEvaluators) {
        bool isOk;
        double val = ch->evaluate(rollingEngine,&isOk);
        if(isOk)
            data.append(","+QString::number(val));
        else
            data.append(",-");
    }
    data.append(";");
    emit sendManualInput(data.toLocal8Bit());
}

void ManualInputDialog::on_lineEditManualInput_returnPressed() {
    QByteArray bytes;
    bytes.append(ui->lineEditManualInput->text().toLocal8Bit());
    emit sendManualInput(bytes);
    ui->lineEditManualInput->clear();
    on_pushButtonScrollDown_2_clicked();
}

void ManualInputDialog::on_pushButtonScrollDown_2_clicked() {
    QScrollBar* scroll = ui->plainTextEditConsole_2->verticalScrollBar();
    scroll->setValue(scroll->maximum());
    scroll = ui->plainTextEditConsole_2->horizontalScrollBar();
    scroll->setValue(scroll->minimum());
}


void ManualInputDialog::on_tableWidgetRollingSetup_cellChanged(int row, int column)
{
    if(column==0) {
        auto txt = ui->tableWidgetRollingSetup->item(row,column)->text();
        bool ok = rollingChannelEvaluators.at(row)->setExpression(rollingEngine, txt);
        QTableWidgetItem *item = new QTableWidgetItem(ok?"OK":txt.isEmpty()?"":"Error");
        item->setFlags(item->flags() &  ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        ui->tableWidgetRollingSetup->setItem(row,1,item);
    }
}

void ManualInputDialog::setRollingExprRows(int rows)
{
    while(ui->tableWidgetRollingSetup->rowCount()!=rows) {
        int count = ui->tableWidgetRollingSetup->rowCount();
        if(rows>count) {
            ui->tableWidgetRollingSetup->insertRow(count-1);
            rollingChannelEvaluators.append(new VariableExpressionParser());
        }
        else {
            ui->tableWidgetRollingSetup->removeRow(count-2);
            delete rollingChannelEvaluators.last();
            rollingChannelEvaluators.removeLast();
        }
    }

    QStringList names;
    for(int i = 1; i < ui->tableWidgetRollingSetup->rowCount();i++)
        names.append(QString("Ch%1").arg(i));
    names.append("");
    ui->tableWidgetRollingSetup->setVerticalHeaderLabels(names);
}

