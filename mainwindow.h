#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

#include "creditem.h"

class CredItemWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QVector<creditem> creds, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_searchButton_clicked();
    void on_viewButton_clicked();
    void onCredWidgetClicked(CredItemWidget* widget);


private:
    void buildCredWidgets();
    void filterByUrl(const QString& text);

    Ui::MainWindow *ui = nullptr;
    QVector<creditem> creds_;
    QVector<CredItemWidget*> widgets_;
    CredItemWidget* m_selected = nullptr;
    // CredItemWidget* m_selectedWidget = nullptr;
    // int m_selectedIndex = -1;
};

#endif
