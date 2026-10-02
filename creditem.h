#ifndef CREDITEM_H
#define CREDITEM_H

#include <QString>
#include <QVector>
#include <QByteArray>

class creditem
{
public:
    creditem() = default;
    creditem(QString url, QString login, QString password);

    const QString& url() const;

    QString decryptLogin(const QString& pin) const;
    QString decryptPassword(const QString& pin) const;

    void encryptInMemory(const QByteArray& memKey);

    static QVector<creditem> loadFromEncryptedFile(const QString& encPath,
                                                   const QString& pin,
                                                   QString* err = nullptr);

    static QByteArray deriveMemoryKey(const QString& pin);


private:
    QString m_url;
    QByteArray m_encLogin;
    QByteArray m_encPassword;
    QByteArray m_memIv;

    static QByteArray encryptAes(const QByteArray& plaintext, const QByteArray& key, const QByteArray& iv);
    static QByteArray decryptAes(const QByteArray& ciphertext, const QByteArray& key, const QByteArray& iv);
};

#endif
