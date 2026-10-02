#include "creditem.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRandomGenerator>

#include <openssl/evp.h>


creditem::creditem(QString url, QString login, QString password)
    : m_url(std::move(url))
{
    m_encLogin = login.toUtf8();
    m_encPassword = password.toUtf8();
}

const QString& creditem::url() const { return m_url; }


QByteArray creditem::deriveMemoryKey(const QString& pin)
{
    QByteArray data = QByteArray("mem_") + pin.toUtf8();
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}


void creditem::encryptInMemory(const QByteArray& memKey)
{
    m_memIv.resize(16);
    for (int i = 0; i < 16; ++i)
        m_memIv[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));

    QByteArray plainLogin = m_encLogin;
    QByteArray plainPassword = m_encPassword;

    m_encLogin = encryptAes(plainLogin, memKey, m_memIv);
    m_encPassword = encryptAes(plainPassword, memKey, m_memIv);

    plainLogin.fill('\0');
    plainPassword.fill('\0');
}

QString creditem::decryptLogin(const QString& pin) const
{
    QByteArray memKey = deriveMemoryKey(pin);
    QByteArray plain = decryptAes(m_encLogin, memKey, m_memIv);
    if (plain.isEmpty()) return QString();
    return QString::fromUtf8(plain);
}

QString creditem::decryptPassword(const QString& pin) const
{
    QByteArray memKey = deriveMemoryKey(pin);
    QByteArray plain = decryptAes(m_encPassword, memKey, m_memIv);
    if (plain.isEmpty()) return QString();
    return QString::fromUtf8(plain);
}


QByteArray creditem::encryptAes(const QByteArray& plaintext, const QByteArray& key, const QByteArray& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};

    QByteArray ciphertext;
    ciphertext.resize(plaintext.size() + 16);
    int outLen1 = 0, outLen2 = 0;
    bool ok = true;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
        ok = false;

    if (ok && EVP_EncryptUpdate(ctx,
                                reinterpret_cast<unsigned char*>(ciphertext.data()), &outLen1,
                                reinterpret_cast<const unsigned char*>(plaintext.constData()),
                                plaintext.size()) != 1)
        ok = false;

    if (ok && EVP_EncryptFinal_ex(ctx,
                                  reinterpret_cast<unsigned char*>(ciphertext.data() + outLen1),
                                  &outLen2) != 1)
        ok = false;

    EVP_CIPHER_CTX_free(ctx);
    if (!ok) return {};
    ciphertext.resize(outLen1 + outLen2);
    return ciphertext;
}

QByteArray creditem::decryptAes(const QByteArray& ciphertext, const QByteArray& key, const QByteArray& iv)
{
    if (ciphertext.isEmpty() || iv.isEmpty()) return {};

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};

    QByteArray plaintext;
    plaintext.resize(ciphertext.size() + 16);
    int outLen1 = 0, outLen2 = 0;
    bool ok = true;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
        ok = false;

    if (ok && EVP_DecryptUpdate(ctx,
                                reinterpret_cast<unsigned char*>(plaintext.data()), &outLen1,
                                reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                                ciphertext.size()) != 1)
        ok = false;

    if (ok && EVP_DecryptFinal_ex(ctx,
                                  reinterpret_cast<unsigned char*>(plaintext.data() + outLen1),
                                  &outLen2) != 1)
        ok = false;

    EVP_CIPHER_CTX_free(ctx);
    if (!ok) return {};
    plaintext.resize(outLen1 + outLen2);
    return plaintext;
}


static QVector<creditem> parseCredsJson(const QByteArray& jsonBytes, QString* err)
{
    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &pe);

    if (pe.error != QJsonParseError::NoError) {
        if (err) *err = "Ошибка парсинга JSON: " + pe.errorString();
        return {};
    }
    if (!doc.isObject()) {
        if (err) *err = "JSON должен быть объектом верхнего уровня.";
        return {};
    }

    const QJsonObject root = doc.object();
    const QJsonValue credsVal = root.value("creds");
    if (!credsVal.isArray()) {
        if (err) *err = "В JSON нет массива 'creds'.";
        return {};
    }

    QVector<creditem> out;
    const QJsonArray arr = credsVal.toArray();
    out.reserve(arr.size());

    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        const QString url = o.value("url").toString();

        const QJsonValue secretVal = o.value("secret");
        if (!secretVal.isObject()) continue;

        const QJsonObject secret = secretVal.toObject();
        const QString login = secret.value("login").toString();
        const QString password = secret.value("password").toString();

        if (!url.isEmpty())
            out.push_back(creditem(url, login, password));
    }

    return out;
}


QVector<creditem> creditem::loadFromEncryptedFile(const QString& encPath,
                                                  const QString& pin,
                                                  QString* err)
{
    if (err) err->clear();

    QFile f(encPath);
    if (!f.open(QIODevice::ReadOnly)) {
        if (err) *err = "Не удалось открыть creds.enc: " + encPath;
        return {};
    }

    const QByteArray ciphertext = f.readAll();
    f.close();

    if (ciphertext.isEmpty()) {
        if (err) *err = "Файл creds.enc пустой.";
        return {};
    }


    const QByteArray iv = QByteArray::fromHex("00010203040506070809101112131415");


    const QByteArray key = QCryptographicHash::hash(pin.toUtf8(), QCryptographicHash::Sha256);

    QByteArray plaintext;
    plaintext.resize(ciphertext.size() + 16);
    int outLen1 = 0, outLen2 = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        if (err) *err = "EVP_CIPHER_CTX_new() failed.";
        return {};
    }

    bool ok = true;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1)
        ok = false;

    if (ok && EVP_DecryptUpdate(ctx,
                                reinterpret_cast<unsigned char*>(plaintext.data()), &outLen1,
                                reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                                ciphertext.size()) != 1)
        ok = false;

    if (ok && EVP_DecryptFinal_ex(ctx,
                                  reinterpret_cast<unsigned char*>(plaintext.data() + outLen1),
                                  &outLen2) != 1)
        ok = false;

    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        if (err) *err = "Неверный PIN или файл повреждён.";
        return {};
    }

    plaintext.resize(outLen1 + outLen2);

    int start = plaintext.indexOf('{');
    if (start > 0)
        plaintext = plaintext.mid(start);

    QVector<creditem> creds = parseCredsJson(plaintext, err);
    plaintext.fill('\0');

    if (creds.isEmpty()) {
        if (err && err->isEmpty()) *err = "После расшифровки не найдено данных.";
        return {};
    }

    QByteArray memKey = deriveMemoryKey(pin);
    for (auto& c : creds)
        c.encryptInMemory(memKey);
    memKey.fill('\0');

    return creds;
}
