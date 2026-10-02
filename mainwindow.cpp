#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "creditemwidget.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>

// #include <QApplication>
// #include <QClipboard>
// #include <QPushButton>

MainWindow::MainWindow(QVector<creditem> creds, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , creds_(std::move(creds))
{
    ui->setupUi(this);

    buildCredWidgets();

    connect(ui->searchLine, &QLineEdit::textChanged, this, &MainWindow::filterByUrl);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buildCredWidgets()
{
    QWidget* container = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignTop);

    for (int i = 0; i < creds_.size(); ++i) {
        CredItemWidget* w = new CredItemWidget(&creds_[i], container);
        connect(w, &CredItemWidget::clicked, this, &MainWindow::onCredWidgetClicked);
        layout->addWidget(w);
        widgets_.append(w);
    }

    ui->scrollArea->setWidget(container);
}

void MainWindow::onCredWidgetClicked(CredItemWidget* widget)
{

    if (m_selected)
        m_selected->setSelected(false);

    m_selected = widget;
    m_selected->setSelected(true);
}

void MainWindow::filterByUrl(const QString& text)
{
    for (auto* w : widgets_) {
        bool match = text.isEmpty() || w->url().contains(text, Qt::CaseInsensitive);
        w->setVisible(match);
    }
}

void MainWindow::on_searchButton_clicked()
{
    filterByUrl(ui->searchLine->text());
}




void MainWindow::on_viewButton_clicked()
{
    if (!m_selected) {
        QMessageBox::information(this, "Просмотр", "Выберите запись (клик по карточке).");
        return;
    }

    bool ok = false;
    QString pin = QInputDialog::getText(
        this,
        "Введите PIN",
        "Для просмотра учётных данных введите PIN-код:",
        QLineEdit::Password,
        QString(),
        &ok
        );

    if (!ok || pin.isEmpty())
        return;

    m_selected->reveal(pin);
}
