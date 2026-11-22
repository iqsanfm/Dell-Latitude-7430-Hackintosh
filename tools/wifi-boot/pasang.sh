#!/bin/zsh
# Pasang penjaga Wi-Fi saat boot.  Jalankan: sudo zsh pasang.sh   (copot: sudo zsh copot.sh)
[[ $EUID -eq 0 ]] || { print "Jalankan pakai sudo: sudo zsh $0"; exit 1; }
D=${0:A:h}; P=/Library/LaunchDaemons/local.xe.wifiboot.plist
mkdir -p /usr/local/libexec
install -m 755 -o root -g wheel $D/wifi-boot.sh /usr/local/libexec/xe-wifi-boot.sh
install -m 644 -o root -g wheel $D/local.xe.wifiboot.plist $P
launchctl bootout system $P 2>/dev/null
launchctl bootstrap system $P && print "Terpasang (jalan sekali tiap boot). Log: /var/log/xe-wifi-boot.log"
sleep 8; tail -3 /var/log/xe-wifi-boot.log 2>/dev/null
