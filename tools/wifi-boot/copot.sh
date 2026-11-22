#!/bin/zsh
[[ $EUID -eq 0 ]] || { print "Jalankan pakai sudo: sudo zsh $0"; exit 1; }
P=/Library/LaunchDaemons/local.xe.wifiboot.plist
launchctl bootout system $P 2>/dev/null; rm -f $P /usr/local/libexec/xe-wifi-boot.sh; print "Dicopot."
