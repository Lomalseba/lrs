#include <QApplication>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>

#include <windows.h>
#include <winnt.h>

#include "pindialog.h"

// bool checkForDebugger()
// {
//     if (IsDebuggerPresent()) {
//         QMessageBox::critical(nullptr, "Ошибка безопасности",
//                               "Обнаружен отладчик. Программа не может быть запущена.");
//         return true;
//     }
//     return false;
// }

bool verifyTextSectionSha256(QString *errorOut = nullptr)
{

    wchar_t exePath[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, exePath, MAX_PATH)) {
        if (errorOut) *errorOut = "Не удалось получить путь к исполняемому файлу";
        return false;
    }

    QFile file(QString::fromWCharArray(exePath));
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorOut) *errorOut = "Не удалось открыть исполняемый файл";
        return false;
    }
    const QByteArray fileData = file.readAll();
    file.close();

    const auto *base = reinterpret_cast<const unsigned char*>(fileData.constData());

    auto *dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        if (errorOut) *errorOut = "Некорректная DOS-сигнатура";
        return false;
    }

    auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        if (errorOut) *errorOut = "Некорректная NT-сигнатура";
        return false;
    }

    const IMAGE_SECTION_HEADER *sec = IMAGE_FIRST_SECTION(nt);
    const WORD secCount = nt->FileHeader.NumberOfSections;
    const IMAGE_SECTION_HEADER *textSec = nullptr;

    for (WORD i = 0; i < secCount; ++i) {
        char name[9]{};
        memcpy(name, sec[i].Name, 8);
        if (strcmp(name, ".text") == 0) {
            textSec = &sec[i];
            break;
        }
    }

    if (!textSec) {
        if (errorOut) *errorOut = "Не найдена секция .text";
        return false;
    }

    const DWORD virtualAddress = textSec->VirtualAddress;
    const DWORD virtualSize    = textSec->Misc.VirtualSize;

    const DWORD rawOffset = textSec->PointerToRawData;
    const DWORD rawSize   = textSec->SizeOfRawData;

    if (rawOffset + rawSize > static_cast<DWORD>(fileData.size())) {
        if (errorOut) *errorOut = "Некорректные данные секции .text в файле";
        return false;
    }

    qDebug() << "Виртуальный адрес .text: 0x" + QByteArray::number(virtualAddress, 16).toUpper();
    qDebug() << "Размер .text (VirtualSize):" << virtualSize << "байт";

    const QByteArray textData(fileData.constData() + rawOffset, rawSize);
    const QByteArray calculatedHash = QCryptographicHash::hash(textData, QCryptographicHash::Sha256);

    qDebug() << "Calculated SHA256:" << calculatedHash.toHex();

    const QByteArray referenceHash = QByteArray::fromHex(
        "12d946e6955522324b5b53964cfacfb087551393fc9fb1bcb724da291c701858"
        );

    if (calculatedHash != referenceHash) {
        if (errorOut) *errorOut = QString("Контрольная сумма не совпала!\n"
                                          "Ожидалось: %1\n"
                                          "Получено:  %2")
                                      .arg(QString(referenceHash.toHex()))
                                      .arg(QString(calculatedHash.toHex()));
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // if (checkForDebugger())
    //     return -1;
    QString err;
    if (!verifyTextSectionSha256(&err)) {
        QMessageBox::critical(nullptr, "Ошибка безопасности",
                              "Обнаружена модификация приложения:\n" + err);
        return -1;
    }

    PinDialog dlg;
    dlg.show();

    return app.exec();
}
