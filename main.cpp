#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QString>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QTabWidget>
#include <QRadioButton>
#include <QStackedWidget>
#include <QGroupBox>
#include <QtMath>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>

// Header untuk JSON Storage & Autocomplete bawaan Qt Core
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCompleter>
#include <QStandardItemModel>
#include <QMap>

#include "raw_printer.h"

const QString JSON_FILE_NAME = "523_database.json";

// Lokasi database di folder data user (AppData / Application Support / ~/.local/share),
// supaya tetap bisa ditulis walau app diekstrak di mana saja dan aman saat ganti versi.
QString databasePath() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    QString path = QDir(dir).filePath(JSON_FILE_NAME);

    // Migrasi: salin database lama dari folder kerja kalau di lokasi baru belum ada
    if (!QFile::exists(path) && QFile::exists(JSON_FILE_NAME)) {
        QFile::copy(JSON_FILE_NAME, path);
    }
    return path;
}

int mmToDots(double mm) {
    return qRound((mm * 203.0) / 25.4);
}

// Fungsi load data dari file JSON
QMap<QString, QString> loadJsonData() {
    QMap<QString, QString> dataMap;
    QFile file(databasePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return dataMap; // Kembalikan map kosong jika file belum ada
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject jsonObj = doc.object();

    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        dataMap[it.key()] = it.value().toString();
    }
    return dataMap;
}

// Fungsi simpan/update data ke file JSON (Auto-Save)
void saveToJson(const QString &b1, const QString &b2) {
    if (b1.trimmed().isEmpty()) return;

    QMap<QString, QString> currentData = loadJsonData();
    currentData[b1.trimmed()] = b2.trimmed();

    QJsonObject jsonObj;
    for (auto it = currentData.begin(); it != currentData.end(); ++it) {
        jsonObj.insert(it.key(), it.value());
    }

    QJsonDocument doc(jsonObj);
    QFile file(databasePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

// Fungsi refresh drop-down rekomendasi pencarian (Autocomplete)
void refreshCompleterFromJson(QCompleter *completer, QStandardItemModel *model, QMap<QString, QString> &dataMap) {
    model->clear();
    dataMap = loadJsonData();

    for (auto it = dataMap.begin(); it != dataMap.end(); ++it) {
        model->appendRow(new QStandardItem(it.key()));
    }
    completer->setModel(model);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setOrganizationName("523");
    app.setApplicationName("523LabelApp");
    QSettings settings;

    QWidget window;
    window.setWindowTitle("523 Label App");

    QTabWidget *tabWidget = new QTabWidget();
    QWidget *tabCetak = new QWidget();
    QWidget *tabSettings = new QWidget();

    // ==========================================
    // TAB SETTINGS
    // ==========================================
    QFormLayout *settingsLayout = new QFormLayout(tabSettings);

    QLineEdit *printerEdit = new QLineEdit(settings.value("printer", DEFAULT_PRINTER_NAME).toString());
    
    QSpinBox *darknessSpin = new QSpinBox();
    darknessSpin->setRange(0, 100);
    darknessSpin->setValue(settings.value("darkness", 30).toInt());
    
    QDoubleSpinBox *pageMarginLeftSpin = new QDoubleSpinBox();
    pageMarginLeftSpin->setSuffix(" mm");
    pageMarginLeftSpin->setValue(settings.value("pageMarginLeft", 2.0).toDouble());

    QDoubleSpinBox *labelWidthSpin = new QDoubleSpinBox();
    labelWidthSpin->setSuffix(" mm");
    labelWidthSpin->setValue(settings.value("labelWidth", 32.0).toDouble());

    QDoubleSpinBox *gapXSpin = new QDoubleSpinBox();
    gapXSpin->setSuffix(" mm");
    gapXSpin->setValue(settings.value("gapX", 3.0).toDouble());

    QDoubleSpinBox *intMarginLeftSpin = new QDoubleSpinBox();
    intMarginLeftSpin->setSuffix(" mm");
    intMarginLeftSpin->setValue(settings.value("intMarginLeft", 1.0).toDouble());

    QDoubleSpinBox *intMarginRightSpin = new QDoubleSpinBox();
    intMarginRightSpin->setSuffix(" mm");
    intMarginRightSpin->setValue(settings.value("intMarginRight", 1.0).toDouble());

    QDoubleSpinBox *internalMarginYSpin = new QDoubleSpinBox();
    internalMarginYSpin->setSuffix(" mm");
    internalMarginYSpin->setValue(settings.value("intMarginY", 2.0).toDouble());

    QPushButton *btnImportCsv = new QPushButton("Import Excel (File .csv)");
    
    settingsLayout->addRow("Nama Printer:", printerEdit);
    settingsLayout->addRow("Ketebalan Tulisan / Darkness:", darknessSpin);
    settingsLayout->addRow("Margin Paling Kiri (Kiri -> L1):", pageMarginLeftSpin);
    settingsLayout->addRow("Lebar 1 Kertas Label:", labelWidthSpin);
    settingsLayout->addRow("Spacing Antar Kertas (L1->L2):", gapXSpin);
    settingsLayout->addRow("Margin Kiri DALAM Kertas:", intMarginLeftSpin);
    settingsLayout->addRow("Margin Kanan DALAM Kertas:", intMarginRightSpin);
    settingsLayout->addRow("Margin Atas DALAM Kertas:", internalMarginYSpin);
    settingsLayout->addRow("Database Option:", btnImportCsv);

    // ==========================================
    // TAB CETAK & AUTOCMPLETE
    // ==========================================
    QVBoxLayout *cetakLayout = new QVBoxLayout(tabCetak);

    QHBoxLayout *modeLayout = new QHBoxLayout();
    QRadioButton *rbSama = new QRadioButton("3 Label Sama");
    QRadioButton *rbBeda = new QRadioButton("3 Label Beda");
    modeLayout->addWidget(rbSama);
    modeLayout->addWidget(rbBeda);
    
    if (settings.value("modeBeda", false).toBool()) rbBeda->setChecked(true);
    else rbSama->setChecked(true);

    QStackedWidget *stackedForm = new QStackedWidget();

    QMap<QString, QString> dbDataMap;
    QStandardItemModel *completerModel = new QStandardItemModel();

    // -- Form SAMA --
    QWidget *widgetSama = new QWidget();
    QFormLayout *layoutSama = new QFormLayout(widgetSama);
    QLineEdit *samaB1 = new QLineEdit(settings.value("samaB1", "").toString());
    QLineEdit *samaB2 = new QLineEdit(settings.value("samaB2", "").toString());
    
    QCompleter *completerSama = new QCompleter();
    completerSama->setCaseSensitivity(Qt::CaseInsensitive);
    completerSama->setFilterMode(Qt::MatchContains);
    samaB1->setCompleter(completerSama);

    layoutSama->addRow("Baris 1:", samaB1);
    layoutSama->addRow("Baris 2:", samaB2);
    stackedForm->addWidget(widgetSama);

    // -- Form BEDA --
    QWidget *widgetBeda = new QWidget();
    QVBoxLayout *layoutBeda = new QVBoxLayout(widgetBeda);
    
    QGroupBox *gbL1 = new QGroupBox("Label 1 (Kiri)");
    QFormLayout *flL1 = new QFormLayout(gbL1);
    QLineEdit *l1B1 = new QLineEdit(settings.value("l1B1", "").toString());
    QLineEdit *l1B2 = new QLineEdit(settings.value("l1B2", "").toString());
    QCompleter *completerL1 = new QCompleter();
    completerL1->setCaseSensitivity(Qt::CaseInsensitive); completerL1->setFilterMode(Qt::MatchContains);
    l1B1->setCompleter(completerL1);
    flL1->addRow("Baris 1:", l1B1); flL1->addRow("Baris 2:", l1B2);
    layoutBeda->addWidget(gbL1);

    QGroupBox *gbL2 = new QGroupBox("Label 2 (Tengah)");
    QFormLayout *flL2 = new QFormLayout(gbL2);
    QLineEdit *l2B1 = new QLineEdit(settings.value("l2B1", "").toString());
    QLineEdit *l2B2 = new QLineEdit(settings.value("l2B2", "").toString());
    QCompleter *completerL2 = new QCompleter();
    completerL2->setCaseSensitivity(Qt::CaseInsensitive); completerL2->setFilterMode(Qt::MatchContains);
    l2B1->setCompleter(completerL2);
    flL2->addRow("Baris 1:", l2B1); flL2->addRow("Baris 2:", l2B2);
    layoutBeda->addWidget(gbL2);

    QGroupBox *gbL3 = new QGroupBox("Label 3 (Kanan)");
    QFormLayout *flL3 = new QFormLayout(gbL3);
    QLineEdit *l3B1 = new QLineEdit(settings.value("l3B1", "").toString());
    QLineEdit *l3B2 = new QLineEdit(settings.value("l3B2", "").toString());
    QCompleter *completerL3 = new QCompleter();
    completerL3->setCaseSensitivity(Qt::CaseInsensitive); completerL3->setFilterMode(Qt::MatchContains);
    l3B1->setCompleter(completerL3);
    flL3->addRow("Baris 1:", l3B1); flL3->addRow("Baris 2:", l3B2);
    layoutBeda->addWidget(gbL3);
    
    stackedForm->addWidget(widgetBeda);

    // Muat data awal rekomendasi dari file JSON
    refreshCompleterFromJson(completerSama, completerModel, dbDataMap);
    completerL1->setModel(completerModel);
    completerL2->setModel(completerModel);
    completerL3->setModel(completerModel);

    auto handleCompletion = [&](const QString &text, QLineEdit *b2Target) {
        if(dbDataMap.contains(text)) {
            b2Target->setText(dbDataMap[text]);
        }
    };
    QObject::connect(completerSama, qOverload<const QString &>(&QCompleter::activated), [=](const QString &text){ handleCompletion(text, samaB2); });
    QObject::connect(completerL1, qOverload<const QString &>(&QCompleter::activated), [=](const QString &text){ handleCompletion(text, l1B2); });
    QObject::connect(completerL2, qOverload<const QString &>(&QCompleter::activated), [=](const QString &text){ handleCompletion(text, l2B2); });
    QObject::connect(completerL3, qOverload<const QString &>(&QCompleter::activated), [=](const QString &text){ handleCompletion(text, l3B2); });

    QObject::connect(rbSama, &QRadioButton::toggled, [&]{ if(rbSama->isChecked()) stackedForm->setCurrentIndex(0); });
    QObject::connect(rbBeda, &QRadioButton::toggled, [&]{ if(rbBeda->isChecked()) stackedForm->setCurrentIndex(1); });
    stackedForm->setCurrentIndex(rbBeda->isChecked() ? 1 : 0);

    QHBoxLayout *rowsLayout = new QHBoxLayout();
    QLabel *lblRows = new QLabel("Jumlah Baris Cetak (Ke Bawah):");
    QSpinBox *rowsSpin = new QSpinBox();
    rowsSpin->setMinimum(1); rowsSpin->setMaximum(9999);
    rowsSpin->setValue(settings.value("rows", 1).toInt());
    rowsLayout->addWidget(lblRows); rowsLayout->addWidget(rowsSpin);

    QPushButton *btnGenerate = new QPushButton("PRINT LABEL");
    btnGenerate->setMinimumHeight(50);

    cetakLayout->addLayout(modeLayout);
    cetakLayout->addWidget(stackedForm);
    cetakLayout->addLayout(rowsLayout);
    cetakLayout->addStretch();
    cetakLayout->addWidget(btnGenerate);

    tabWidget->addTab(tabCetak, "Main Cetak");
    tabWidget->addTab(tabSettings, "Settings");

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    mainLayout->addWidget(tabWidget);

    // LOGIKA IMPORT CSV KE JSON
    QObject::connect(btnImportCsv, &QPushButton::clicked, [&]() {
        QString path = QFileDialog::getOpenFileName(nullptr, "Import CSV Hasil Save As Excel", "", "CSV Files (*.csv)");
        if(path.isEmpty()) return;

        QFile file(path);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, "Error", "Gagal membuka file!");
            return;
        }

        QTextStream in(&file);
        QMap<QString, QString> currentData = loadJsonData();
        int importCount = 0;
        
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(line.contains(';') ? ';' : ',');
            if(fields.size() >= 2) {
                QString col1 = fields.at(0).trimmed().remove('"');
                QString col2 = fields.at(1).trimmed().remove('"');
                if(!col1.isEmpty()) {
                    currentData[col1] = col2;
                    importCount++;
                }
            }
        }
        file.close();

        // Tulis semua kumpulan data baru sekaligus ke file JSON
        QJsonObject jsonObj;
        for (auto it = currentData.begin(); it != currentData.end(); ++it) {
            jsonObj.insert(it.key(), it.value());
        }
        QJsonDocument doc(jsonObj);
        QFile outFile(databasePath());
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(doc.toJson());
            outFile.close();
        }

        refreshCompleterFromJson(completerSama, completerModel, dbDataMap);
        QMessageBox::information(nullptr, "Sukses", QString("%1 Data berhasil diimport ke Database JSON!").arg(importCount));
    });

    // LOGIKA PRINT & AUTO SAVE
    QObject::connect(btnGenerate, &QPushButton::clicked, [&]() {
        settings.setValue("printer", printerEdit->text());
        settings.setValue("darkness", darknessSpin->value());
        settings.setValue("pageMarginLeft", pageMarginLeftSpin->value());
        settings.setValue("labelWidth", labelWidthSpin->value());
        settings.setValue("gapX", gapXSpin->value());
        settings.setValue("intMarginLeft", intMarginLeftSpin->value());
        settings.setValue("intMarginRight", intMarginRightSpin->value());
        settings.setValue("intMarginY", internalMarginYSpin->value());
        
        settings.setValue("modeBeda", rbBeda->isChecked());
        settings.setValue("samaB1", samaB1->text()); settings.setValue("samaB2", samaB2->text());
        settings.setValue("l1B1", l1B1->text()); settings.setValue("l1B2", l1B2->text());
        settings.setValue("l2B1", l2B1->text()); settings.setValue("l2B2", l2B2->text());
        settings.setValue("l3B1", l3B1->text()); settings.setValue("l3B2", l3B2->text());
        settings.setValue("rows", rowsSpin->value());

        // Auto Save Inputan Baru ke JSON
        if(rbSama->isChecked()) {
            saveToJson(samaB1->text(), samaB2->text());
        } else {
            saveToJson(l1B1->text(), l1B1->text().isEmpty() ? "" : l1B2->text());
            saveToJson(l2B1->text(), l2B1->text().isEmpty() ? "" : l2B2->text());
            saveToJson(l3B1->text(), l3B1->text().isEmpty() ? "" : l3B2->text());
        }
        refreshCompleterFromJson(completerSama, completerModel, dbDataMap);

        int pMlDots = mmToDots(pageMarginLeftSpin->value());
        int lwDots  = mmToDots(labelWidthSpin->value());
        int gxDots  = mmToDots(gapXSpin->value());
        int iMlDots = mmToDots(intMarginLeftSpin->value());
        int iMrDots = mmToDots(intMarginRightSpin->value());
        int iMyDots = mmToDots(internalMarginYSpin->value());

        int printAreaWidthDots = lwDots - iMlDots - iMrDots; 
        if(printAreaWidthDots < 10) printAreaWidthDots = lwDots;

        QString zpl = "";

        for (int r = 0; r < rowsSpin->value(); r++) {
            zpl += "^XA\n";
            zpl += "^POI\n";
            zpl += QString("^PW%1\n").arg(pMlDots + (3 * lwDots) + (2 * gxDots) + 16); 
            zpl += QString("~SD%1\n").arg(darknessSpin->value());
            
            for (int c = 0; c < 3; c++) {
                QString b1, b2;
                if (rbSama->isChecked()) {
                    b1 = samaB1->text(); b2 = samaB2->text();
                } else {
                    if (c == 0) { b1 = l1B1->text(); b2 = l1B2->text(); }
                    else if (c == 1) { b1 = l2B1->text(); b2 = l2B2->text(); }
                    else { b1 = l3B1->text(); b2 = l3B2->text(); }
                }

                int offsetX = pMlDots + (c * (lwDots + gxDots)) + iMlDots;
                int offsetY = iMyDots;

                int fs1 = (b1.length() > 15) ? 20 : 25;
                int fs2 = (b2.length() > 15) ? 20 : 25;

                zpl += QString("^FO%1,%2^A0N,%3,%3^FB%4,2,0,C^FD%5^FS\n")
                        .arg(offsetX).arg(offsetY).arg(fs1).arg(printAreaWidthDots).arg(b1);
                
                zpl += QString("^FO%1,%2^A0N,%3,%3^FB%4,2,0,C^FD%5^FS\n")
                        .arg(offsetX).arg(offsetY + 40).arg(fs2).arg(printAreaWidthDots).arg(b2);
            }
            zpl += "^XZ\n";
        }

        QString error = sendRawToPrinter(printerEdit->text(), zpl.toUtf8());
        if(!error.isEmpty()) {
            QMessageBox::warning(nullptr, "Print Error", "Terjadi error: \n" + error);
        }
    });

    window.resize(450, 600);
    window.show();

    return app.exec();
}
