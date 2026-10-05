#include "raw_printer.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <winspool.h>
#include <string>
#include <vector>
#else
#include <QProcess>
#include <QStringList>
#endif

#ifdef Q_OS_WIN

static QString lastWindowsError(const QString &step) {
    DWORD code = GetLastError();
    wchar_t *buffer = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   nullptr, code, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    QString message = buffer ? QString::fromWCharArray(buffer).trimmed() : QString();
    LocalFree(buffer);
    return QString("%1 gagal (kode %2): %3").arg(step).arg(code).arg(message);
}

static QString defaultWindowsPrinter() {
    DWORD size = 0;
    GetDefaultPrinterW(nullptr, &size);
    if (size == 0) return QString();
    std::vector<wchar_t> buffer(size);
    if (!GetDefaultPrinterW(buffer.data(), &size)) return QString();
    return QString::fromWCharArray(buffer.data());
}

static QString writeRawDocument(HANDLE handle, const QByteArray &data) {
    wchar_t docName[] = L"523 Label";
    wchar_t dataType[] = L"RAW";
    DOC_INFO_1W docInfo{docName, nullptr, dataType};

    if (StartDocPrinterW(handle, 1, reinterpret_cast<LPBYTE>(&docInfo)) == 0) {
        return lastWindowsError("Memulai dokumen cetak");
    }

    QString error;
    if (!StartPagePrinter(handle)) {
        error = lastWindowsError("Memulai halaman");
    } else {
        DWORD written = 0;
        DWORD size = static_cast<DWORD>(data.size());
        if (!WritePrinter(handle, const_cast<char *>(data.constData()), size, &written)) {
            error = lastWindowsError("Mengirim data ke printer");
        } else if (written != size) {
            error = QString("Data terkirim tidak lengkap (%1 dari %2 byte).").arg(written).arg(size);
        }
        EndPagePrinter(handle);
    }
    EndDocPrinter(handle);
    return error;
}

QString sendRawToPrinter(const QString &printerName, const QByteArray &data) {
    QString name = printerName.trimmed();
    if (name.isEmpty()) name = defaultWindowsPrinter();
    if (name.isEmpty()) return "Tidak ada printer default. Isi nama printer di tab Settings.";

    std::wstring wideName = name.toStdWString();
    HANDLE handle = nullptr;
    if (!OpenPrinterW(wideName.data(), &handle, nullptr)) {
        return lastWindowsError(QString("Membuka printer \"%1\"").arg(name))
               + "\nCek nama printer di Settings > Printers & scanners.";
    }

    QString error = writeRawDocument(handle, data);
    ClosePrinter(handle);
    return error;
}

#else

const int PRINT_TIMEOUT_MS = 30000;

QString sendRawToPrinter(const QString &printerName, const QByteArray &data) {
    QStringList args;
    if (!printerName.trimmed().isEmpty()) args << "-d" << printerName.trimmed();
    args << "-o" << "raw";

    QProcess process;
    process.start("lp", args);
    if (!process.waitForStarted()) {
        return "Perintah 'lp' tidak bisa dijalankan. Pastikan CUPS terpasang.\n" + process.errorString();
    }

    process.write(data);
    process.closeWriteChannel();
    if (!process.waitForFinished(PRINT_TIMEOUT_MS)) {
        process.kill();
        return "Printer tidak merespons (timeout).";
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QString error = QString::fromUtf8(process.readAllStandardError()).trimmed();
        return error.isEmpty() ? QString("lp gagal (exit code %1).").arg(process.exitCode()) : error;
    }
    return QString();
}

#endif
