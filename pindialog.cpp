#include "pindialog.h"
#include "ui_pindialog.h"

#include "creditem.h"
#include "mainwindow.h"

#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>

PinDialog::PinDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Form)
{
    ui->setupUi(this);
}

PinDialog::~PinDialog()
{
    delete ui;
}

void PinDialog::on_pinButton_clicked()
{
    const QString pin = ui->pinEnter->text();

    const QString encPath =
        QDir(QCoreApplication::applicationDirPath()).filePath("creds.enc");

    QString err;
    QVector<creditem> creds = creditem::loadFromEncryptedFile(encPath, pin, &err);

    if (creds.isEmpty()) {
        QMessageBox::warning(this, "PIN", err.isEmpty() ? "Неверный PIN" : err);
        ui->pinEnter->clear();
        return;
    }

    auto* w = new MainWindow(std::move(creds));
    w->show();

    accept();
}
