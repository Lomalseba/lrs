#ifndef CREDITEMWIDGET_H
#define CREDITEMWIDGET_H

#include <QWidget>
#include "creditem.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class CredItemForm; }
QT_END_NAMESPACE

class CredItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CredItemWidget(const creditem* item, QWidget *parent = nullptr);
    ~CredItemWidget();

    QString url() const;
    void reveal(const QString& pin);
    void mask();

    void setSelected(bool selected);
    bool isSelected() const;

signals:
    void clicked(CredItemWidget* widget);



protected:
    void mousePressEvent(QMouseEvent* event) override;


private:
    Ui::CredItemForm *ui;
    const creditem* m_item;
    bool m_selected = false;

    void setCopyButtonsVisible(bool visible);
    void copyLogin();
    void copyPassword();

    QString m_plainLogin;
    QString m_plainPassword;
    QTimer* m_maskTimer = nullptr;

};

#endif
