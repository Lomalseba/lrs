#include "creditemwidget.h"
#include "ui_CredItem.h"

#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

CredItemWidget::CredItemWidget(const creditem* item, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CredItemForm)
    , m_item(item)
{
    ui->setupUi(this);
    ui->lineEditlogin->setReadOnly(true);
    ui->lineEditpassword->setReadOnly(true);


    ui->labelURL->setText(m_item ? m_item->url() : QString());


    mask();
    setCopyButtonsVisible(false);


    connect(ui->pushButtonCopyLogin, &QPushButton::clicked, this, [this]() { copyLogin(); });
    connect(ui->pushButtonCopyPassword, &QPushButton::clicked, this, [this]() { copyPassword(); });


    ui->labelURL->setText(m_item->url());
    ui->lineEditlogin->setText("******");
    ui->lineEditlogin->setReadOnly(true);
    ui->lineEditpassword->setText("******");
    ui->lineEditpassword->setReadOnly(true);

    m_maskTimer = new QTimer(this);
    m_maskTimer->setSingleShot(true);

    connect(m_maskTimer, &QTimer::timeout, this, [this]() {
        mask();
    });
}

CredItemWidget::~CredItemWidget()
{
    delete ui;
}

QString CredItemWidget::url() const
{
    return m_item->url();
}

void CredItemWidget::reveal(const QString& pin)
{
    if (!m_item) return;

    const QString login = m_item->decryptLogin(pin);
    const QString pass  = m_item->decryptPassword(pin);

    if (login.isEmpty() && pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Неверный PIN-код.");
        return;
    }

    m_plainLogin = login;
    m_plainPassword = pass;

    ui->lineEditlogin->setText(m_plainLogin);
    ui->lineEditpassword->setText(m_plainPassword);

    setCopyButtonsVisible(true);

    m_maskTimer->start(10000);
}


void CredItemWidget::mask()
{
    m_plainLogin.clear();
    m_plainPassword.clear();

    ui->lineEditlogin->setText("********");
    ui->lineEditpassword->setText("********");

    setCopyButtonsVisible(false);
}


void CredItemWidget::setSelected(bool selected)
{
    m_selected = selected;
    if (selected)
        setStyleSheet("background-color: #696969;");
    else
        setStyleSheet("");
}

bool CredItemWidget::isSelected() const
{
    return m_selected;
}

void CredItemWidget::mousePressEvent(QMouseEvent* event)
{
    emit clicked(this);
    QWidget::mousePressEvent(event);
}


void CredItemWidget::setCopyButtonsVisible(bool visible)
{
    ui->pushButtonCopyLogin->setVisible(visible);
    ui->pushButtonCopyPassword->setVisible(visible);
}

void CredItemWidget::copyLogin()
{
    if (!m_plainLogin.isEmpty())
        QApplication::clipboard()->setText(m_plainLogin);
}

void CredItemWidget::copyPassword()
{
    if (!m_plainPassword.isEmpty())
        QApplication::clipboard()->setText(m_plainPassword);
}


