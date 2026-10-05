# 523 Label App

> **Download aplikasi:** [Releases](https://github.com/Kevinsorensen523/523LabelApp/releases/latest) (Windows & Mac)
>
> **Pasang printer Zebra di Mac** (colok printer dulu), buka Terminal, paste, Enter:
>
> ```bash
> curl -fsSL https://raw.githubusercontent.com/Kevinsorensen523/523LabelApp/main/packaging/Pasang-Printer.command | bash
> ```

A native C++ desktop application built with the Qt framework for automated thermal label printing. This application interfaces directly with thermal printers via the CUPS subsystem using dynamic Zebra Programming Language (ZPL) injection. It includes a localized JSON database configuration, contextual autocompletion (`QCompleter`), and a robust Excel-to-CSV data ingestion pipeline.

---

## 1. System Requirements & Prerequisites

The application is designed for Linux (Ubuntu-based environments) and supports industrial thermal printers such as Zebra ZD220 (203 DPI).

### Dependencies

- GCC / Build Essentials
- CMake
- Qt5 / Qt6 Development Libraries
- CUPS Printing System

### Printer Setup

The application sends raw ZPL to the printer. The transport depends on the OS (see `raw_printer.cpp`).

**Linux / macOS (CUPS, via `lp -o raw`)**
- Find the USB URI: `lpinfo -v | grep -i zebra`
- Linux: add a raw queue: `sudo lpadmin -p ZTC-ZD220-203dpi-ZPL -E -v "<usb-uri>" -m raw`
- macOS (raw queues are rejected): run `packaging/Pasang-Printer.command`, which uses the built-in `Generic.ppd`; the app's `lp -o raw` still bypasses the driver
- macOS deps: `brew install cmake qt`, then build with `-DCMAKE_PREFIX_PATH="$(brew --prefix qt)"`

**Windows (Print Spooler, RAW datatype)**
- Install the Zebra **ZDesigner** driver and connect the printer via USB
- Use the exact name shown in *Settings → Printers & scanners* (default: `ZDesigner ZD220-203dpi ZPL`)
- Build with Qt6 for Windows (MSVC or MinGW); `winspool` is linked automatically

On all platforms, leaving the printer name empty prints to the system default printer.

---

## 2. Build & Compilation

The project uses CMake as its build system.

Steps:
- Navigate to project directory
- Create a build directory
- Generate build files using CMake
- Compile using system toolchain

---

## 3. Running the Application

The application can be executed in two modes:

### Local Mode
Run directly on the Ubuntu desktop environment.

### Remote Mode (SSH with GUI Forwarding)
Run the application remotely while rendering the GUI on the host machine display (`DISPLAY=:0`).

---

## 4. Application Features

The application is divided into two main tabs:

---

### Tab 1: Main Cetak (Operational Interface)

Used for daily label printing operations.

#### Print Modes

**3 Label Sama**
- One input is duplicated across all three label columns
- Suitable for identical SKU printing

**3 Label Beda**
- Each column is independent
- Supports 3 different SKUs in a single row (Left / Middle / Right)

---

#### Contextual Autocomplete

- Powered by `QCompleter`
- Filters local JSON database in real time
- Selecting a suggestion auto-fills related fields

---

#### Auto Save System

- New entries are automatically stored when printing
- No manual save required
- Data is mapped as key-value pairs into local JSON database

---

#### Print Volume Control

- Controls number of rows printed vertically
- Each row contains 3 labels (depending on layout mode)
- Total output scales based on row multiplier

---

### Tab 2: Settings (Hardware & System Configuration)

All settings persist using Qt's `QSettings`.

#### Printer Configuration

- Printer name must match CUPS configuration

#### Print Calibration

- Darkness / Density (ZPL print intensity)
- Left margin offset
- Label width calibration
- Spacing between labels
- Internal margin controls (top/left/right)

---

## 5. Data Architecture

### Local Database (JSON)

The system uses a lightweight JSON file as its primary storage engine.

Structure:
- Key: Product / SKU name
- Value: Product variant / attributes

---

### Excel / CSV Import

Supports bulk import from spreadsheet systems.

Format:
- Column A: Product / SKU
- Column B: Variant / Attribute

Import flow:
- Export spreadsheet to CSV format
- Load file via Settings import feature
- System parses and cleans data
- Data is merged into JSON database
- Autocomplete index is refreshed automatically

---

## 6. Technical Architecture

### Core Stack

- C++17
- Qt Widgets (GUI)
- CMake build system
- Native Linux CUPS integration

---

### Design Principles

- No external database dependency
- Fully JSON-based local storage
- Direct ZPL command generation
- Minimal and lightweight architecture
- Offline-first industrial tool

---

### Print Pipeline Flow

UI Input → Validation → JSON Lookup → ZPL Generator → CUPS → Printer

---

### Hardware Communication

- ZPL is generated dynamically based on layout calculations
- Unit conversion from millimeters to printer dots
- Raw stream sent directly to CUPS printing system

---

## 7. Project Structure

523LabelApp/
├── CMakeLists.txt
├── main.cpp
├── 523_database.json
├── build/
└── README.md

---

## 8. Development Notes

This project is designed as:

- Industrial label printing tool
- Offline-first desktop application
- Lightweight embedded-ready system
- Extensible architecture for future automation

---

## 9. License

MIT License

Copyright (c) 2026 523 Label App

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The software is provided "as is", without warranty of any kind.

---

## 10. Future Improvements

- Visual label preview engine
- Drag & drop template designer
- Multi-printer queue management
- Cloud sync optional database mode
- Barcode / QR module expansion