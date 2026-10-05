#pragma once

#include <QByteArray>
#include <QString>

// Nama printer default per OS:
// - Linux/Mac: nama antrean CUPS
// - Windows: nama printer di "Printers & scanners" (driver ZDesigner)
#ifdef Q_OS_WIN
const QString DEFAULT_PRINTER_NAME = "ZDesigner ZD220-203dpi ZPL";
#else
const QString DEFAULT_PRINTER_NAME = "ZTC-ZD220-203dpi-ZPL";
#endif

// Kirim data mentah (ZPL) langsung ke printer tanpa diproses driver.
// printerName kosong = pakai printer default sistem.
// Return string kosong kalau sukses, atau pesan error kalau gagal.
QString sendRawToPrinter(const QString &printerName, const QByteArray &data);
