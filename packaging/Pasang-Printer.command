#!/bin/bash
# Daftarkan printer Zebra (USB) ke Mac sebagai antrean raw untuk 523 Label App.
# Cukup klik dua kali file ini.

PRINTER_NAME="ZTC-ZD220-203dpi-ZPL"
# macOS menolak antrean raw (-m raw); pakai driver generik bawaan.
# App tetap kirim ZPL mentah lewat "lp -o raw", jadi driver ini dilewati saat cetak.
GENERIC_PPD="/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/PrintCore.framework/Versions/A/Resources/Generic.ppd"

pause_and_exit() {
    echo
    read -r -p "Tekan Enter untuk menutup..."
    exit "$1"
}

echo "Mencari printer Zebra yang tersambung via USB..."
URI=$(lpinfo -v 2>/dev/null | awk '/usb:\/\/Zebra/ {print $2; exit}')

if [ -z "$URI" ]; then
    echo "Printer Zebra tidak ditemukan."
    echo "Pastikan printer sudah dicolok USB dan dinyalakan, lalu jalankan file ini lagi."
    pause_and_exit 1
fi

echo "Ketemu: $URI"
echo
echo "Masukkan password Mac kamu (tidak kelihatan saat diketik, itu normal):"
if sudo lpadmin -p "$PRINTER_NAME" -E -v "$URI" -P "$GENERIC_PPD"; then
    echo
    echo "BERHASIL. Printer terdaftar dengan nama: $PRINTER_NAME"
    echo "Sekarang buka 523LabelApp dan langsung cetak."
    pause_and_exit 0
fi

echo
echo "Gagal mendaftarkan printer. Screenshot jendela ini dan kirim ke admin."
pause_and_exit 1
