#!/bin/zsh
# xe-wifi-boot v2 — sekali jalan per boot (LaunchDaemon local.xe.wifiboot). Log: /var/log/xe-wifi-boot.log
# Kalau ForbiddenItlwm tidak sampai SSM_EVENT_FW_LOADED (bug upstream #20: init firmware tak ALIVE,
# SCAN_MANAGER HALTED sampai Wi-Fi di-toggle): SIMPAN DIAGNOSTIK dulu, baru matikan/nyalakan Wi-Fi.
# v2: (1) cari event cuma di baris kernel — v1 ketipu baris `log` sendiri yang memuat teks predicate-nya
#     (boot 22:43 dianggap "normal" padahal gagal); (2) snapshot diagnostik tiap boot ke /var/log/xe-wifi-boot/
#     (baik-*, gagal-sebelum-toggle-*, gagal-sesudah-toggle-*), simpan 12 terakhir.
IF=en0; LOG=${XE_LOG:-/var/log/xe-wifi-boot.log}; DD=${XE_DD:-/var/log/xe-wifi-boot}
BT=$(sysctl -n kern.boottime | sed -E 's/^\{ sec = ([0-9]+).*/\1/')
START=${XE_START:-$(date -r $BT '+%Y-%m-%d %H:%M:%S')}; BTAG=$(date -j -f '%Y-%m-%d %H:%M:%S' "$START" +%Y%m%d-%H%M%S)
uptime_s() { print $(( $(date +%s) - BT )) }
catat() { print -r -- "$(date '+%F %T') [up $(uptime_s)s] $*" >> $LOG }
klog() { local e=(); [[ -n $XE_END ]] && e=(--end "$XE_END"); /usr/bin/log show --start "$START" $e --style compact --predicate "process == \"kernel\" AND ($1)" 2>/dev/null }
P_SSM='eventMessage CONTAINS "got event: SSM_EVENT_FW_LOADED" OR eventMessage CONTAINS "got event: SSM_EVENT_DRIVER_UNAVAILABLE"'
P_DIAG='eventMessage CONTAINS[c] "itlwm" OR eventMessage CONTAINS "iwx" OR eventMessage CONTAINS "SecBoot" OR eventMessage CONTAINS[c] "init firmware" OR eventMessage CONTAINS[c] "firmware alive" OR eventMessage CONTAINS[c] "ucode" OR eventMessage CONTAINS "SSM_" OR eventMessage CONTAINS "SCAN_MANAGER" OR eventMessage CONTAINS "isDriverAvailable" OR eventMessage CONTAINS "SUPPORTED_CHANNELS" OR eventMessage CONTAINS "ItlAWDL"'
cari() { klog "$P_SSM" | grep -oE 'SSM_EVENT_(FW_LOADED|DRIVER_UNAVAILABLE)' | tail -1 }
snapshot() {  # $1 = label
  local d=$DD/$BTAG-$1; mkdir -p $d
  { print "label=$1  boot=$START  dibuat=$(date '+%F %T')  uptime=$(uptime_s)s"
    sw_vers | paste -sd' ' -; print "boot-args: $(nvram boot-args 2>/dev/null | cut -f2-)"
    kmutil showloaded --list-only 2>/dev/null | grep -iE 'itlwm|BTLEAdv|SMCDellDDV' | awk '{print $6, $7}'
    print "airport power: $(networksetup -getairportpower $IF 2>/dev/null)"
    ifconfig $IF 2>/dev/null | grep -E 'status|inet '
    print "== ioreg"; ioreg -l -w0 | grep -E '"Itlwm[A-Za-z]*"' | sed 's/^[ |]*//' | sort -u
  } > $d/info.txt 2>&1
  ioreg -l -w0 | grep -E '"ITLAWDL[A-Za-z]*"' | sed 's/^[ |]*//' | sort -u > $d/ioreg-itlawdl.txt 2>/dev/null
  klog "$P_DIAG" > $d/kernel.log
  catat "snapshot $1 -> $d ($(wc -l < $d/kernel.log | tr -d ' ') baris kernel)"
  ls -1dt $DD/*/ 2>/dev/null | tail -n +13 | while read -r o; do rm -rf "$o"; done
}
toggle() {
  networksetup -getairportpower $IF 2>/dev/null | grep -q 'On$' || { catat "Wi-Fi memang dimatikan user, dibiarkan"; return 1 }
  catat "$1 -> matikan/nyalakan Wi-Fi"
  networksetup -setairportpower $IF off; sleep 3; networksetup -setairportpower $IF on
}
[[ -n $XE_TEST ]] && { "$@"; exit }   # mode tes: XE_TEST=1 zsh wifi-boot.sh <fungsi> ...
catat "mulai v2 (boot $START)"
(( $(uptime_s) > 240 )) && { catat "dijalankan di tengah boot (pasang ulang?) -> cuma snapshot"; snapshot manual; exit 0 }
n=0
while (( $(uptime_s) < 240 )); do
  sleep 5
  ev=$(cari)
  if [[ $ev == SSM_EVENT_FW_LOADED ]]; then
    if (( n )); then catat "FW_LOADED muncul setelah toggle ke-$n -> beres"; sleep 5; snapshot gagal-sesudah-toggle
    else catat "FW_LOADED normal, tidak perlu apa-apa"; sleep 5; snapshot baik; fi
    exit 0
  fi
  if [[ $ev == SSM_EVENT_DRIVER_UNAVAILABLE ]] || (( $(uptime_s) > 90 + n * 30 )); then
    (( n >= 2 )) && { catat "sudah 2x toggle, masih gagal ($ev) -> menyerah"; snapshot gagal-menyerah; exit 1 }
    (( n )) || snapshot gagal-sebelum-toggle
    toggle "${ev:-tidak ada FW_LOADED}" || exit 0
    n=$(( n + 1 )); sleep 10
  fi
done
catat "waktu habis tanpa FW_LOADED"; snapshot waktu-habis
