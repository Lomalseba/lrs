#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Form; }
QT_END_NAMESPACE

class PinDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PinDialog(QWidget *parent = nullptr);
    ~PinDialog();

private slots:
    void on_pinButton_clicked();

private:
    Ui::Form *ui;
};

#endif
