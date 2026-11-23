# Prüfbericht FeedMate v1.28

Der ESP32-C3 AP verwendet reinen AP-Modus ohne gespeicherte Routerdaten und eine explizite Sendeleistungsanforderung von 17 dBm. PlatformIO-Standardziel ist `esp32c3_supermini`. Die serielle Treiberbestätigung beweist keine tatsächliche Funkabstrahlung; der Nutzer meldet weiterhin, dass mehrere Geräte das Netz nicht sehen. Hardware-RF-Test bleibt offen.

Buildstatus für C3: in dieser Umgebung wegen des bekannten GCC-Assembler-Pfadfehlers 5 nicht abgeschlossen.

---

# Prüfbericht FeedMate v1.27

## v1.27 Zusatzprüfung WLAN-Initialisierung

- ESP32-Hostname wird nun vor dem ersten `WiFi.mode()`-Aufruf gesetzt.
- Ohne Router-Zugangsdaten wird `WIFI_AP` statt `WIFI_AP_STA` verwendet.
- Mit Heimnetz-Zugangsdaten bleibt `WIFI_AP_STA` aktiv; `WiFi.begin()` wartet,
  bis keine Setup-Clients verbunden sind.
- Die Funkabstrahlung ist weiterhin am konkreten Board zu testen. `softAP()`-
  Rückgabewert und Treiberzustand bestätigen keine Beacon-Empfangbarkeit.
- ESP32-C3-SuperMini benötigt ein passendes Board-/Pinprofil; das aktuelle
  C3-DevKitM-Profil ist nicht automatisch eine verifizierte SuperMini-Pinbelegung.

---

# Prüfbericht FeedMate v1.26

## WLAN-Stabilität

Der Nutzer bestätigt, dass das Setup-AP sichtbar ist und eine Verbindung zunächst
gelingt; danach bricht sie ab. Im bisherigen Ablauf wurde der AP früh aktiviert
und `setupWiFi()` schaltete später mit `WiFi.mode(WIFI_OFF)` das Funkmodul aus,
bevor AP+STA neu gestartet wurden. Das trennte eine verbundene Smartphone-
Sitzung. Außerdem konnte die gleichzeitige Heimnetzverbindung einen Funkkanal-
wechsel auslösen.

v1.26 setzt den Hostnamen vor dem Aktivieren der WLAN-Schnittstellen. Der AP
startet anschließend einmalig und das WLAN wird danach nicht mehr ausgeschaltet.
Die Verbindung zum Heimnetz wartet, bis kein Setup-Client mehr angemeldet ist.
Die Wiederherstellung startet einen bereits intakten AP nicht erneut.

| Prüfung | Ergebnis |
|---|---|
| Controller-Regressionstests (82 Assertions; bestehendes Testprogramm ausgeführt) | bestanden |
| JavaScript-Syntax von Geräteoberfläche und Demo | bestanden |
| ESP8266 D1 mini Firmwarebuild nach WLAN-Änderung | bestanden |
| ESP8266 D1 mini LittleFS-Image mit v1.26-Oberfläche | bestanden |
| ESP32-C3 vollständiger Build | durch GCC-Assembler/Windows-Pfadfehler 5 blockiert |
| Smartphone-Verbindung mit v1.26 auf dem Nutzerboard | noch nicht durchgeführt |

Der Ablauf wurde im Code korrigiert; die stabile Funkverbindung ist erst nach
Neuflashen und Test auf dem konkreten Board bestätigt. Treiberstatus allein
beweist keine tatsächliche Funkübertragung.

---

# Prüfbericht FeedMate v1.25

Stand: 23. November 2026

## Neue v1.25-Funktionsprüfung

- Zeitbasierter API-Probelauf setzte bei aktiviertem Hall-Sensor bisher das
  normale Portionsziel und konnte dadurch vor der angeforderten Laufzeit enden.
  Der Endpunkt übergibt nun ausdrücklich das zeitbasierte Ziel `0`; Jam- und
  Umdrehungsschutz bleiben aktiv. Die Kalibrieroberfläche wartet auf den
  Motorabschluss und prüft Fehlerstatus.
- Beim Speichern eines Gruppenplans wurden Geräte ohne verbleibende Zeiten
  übersprungen. Nun werden auch leere Listen übertragen und entfernte Geräte
  löschen alte Zeitpläne.
- Beim Sortieren eines Zeitplans wurden Reservierungen nach Array-Position
  übertragen. Sie werden nun mit der Fütterungszeit, Laufzeit, Futtersorte und
  Portion verknüpft.
- Die Übersicht ließ die AP-Statuskachel leer; sie zeigt nun den gemeldeten
  Firmwarezustand und die Zahl verbundener Clients. Ein Hinweis stellt klar,
  dass diese Abfrage keine tatsächliche Funkabstrahlung prüft.

Der Nutzer bestätigt, dass kein Gerät den AP findet, obwohl der serielle Monitor
`Starts: 2`, `Failures: 0`, die korrekte SSID und `Clients: 0` meldet. Die
Firmware prüft Rückgabewert, Wi-Fi-Modus, konfigurierte IP und SSID. Das belegt
den Treiber-/Konfigurationszustand, aber nicht, dass die HF-Stufe tatsächlich
Beacons aussendet. Deshalb ist AP-Funkfunktion aktuell **nicht bestanden bzw.
nicht verifiziert** und darf nicht als erfolgreicher Hardwaretest gelten.

## Geprüfter Fehlerfall

Der angeschlossene ESP32-C3 auf COM6 wurde unabhängig vom Quellcode geprüft.
esptool erkannte einen ESP32-C3 QFN32 Revision 0.4 mit 4 MB Flash und der MAC
`44:bd:8d:5e:19:e8`. Das vollständige 4-MB-Flashabbild, App-Image,
Bootloader und Partitionstabelle waren lesbar und hatten gültige Prüfsummen.
Die geflashte App stimmte bytegenau mit dem v1.23-Build überein. Die
Coredump-Partition war leer und ein 20-sekündiger Bootmonitor zeigte keine
Reset-Schleife.

Die NVS-Daten bewiesen, dass v1.23 den AP initialisiert hatte. Dort war jedoch
`FeedMate-e8195e8dbd44` gespeichert. Ursache war die Ausgabe des 64-Bit-eFuse-
Wertes in Host-Byte-Reihenfolge. v1.24 liest die kanonische WLAN-MAC und erzeugt
für dieses Board `FeedMate-44bd8d5e19e8`.

## Reparaturen und Sicherheitsprüfung

- Korrekte, getestete MAC-Formatierung für die öffentliche Geräte-ID.
- Historischer Hash-Salt bleibt erhalten; bestehende Admin- und
  Gruppenschlüssel werden durch die Korrektur nicht ungültig.
- AP explizit sichtbar auf Kanal 1, Adresse `192.168.4.1`, maximal vier Clients.
- Zustandsprüfung alle zwei Sekunden anhand von Modus, IP und SSID; automatischer
  Neustart des AP bei Abweichung.
- Kontrollierte WLAN-Neuinitialisierung, damit ein geänderter Hostname vor dem
  Start der Station gesetzt und das Verwaltungs-AP anschließend sicher wieder
  aktiviert wird.
- AP-Zustand, SSID, IP, Clients sowie Start-/Fehlerzähler in der geschützten
  Status-API und Smartphone-Übersicht.
- Native USB-Konsole für ESP32-C3/C6 aktiviert.
- Zeitzonen auf die angebotenen IANA-Namen und bekannte Altdaten begrenzt;
  unbekannte Freitexte werden abgelehnt.
- Sicherheitsheader für HTML ergänzt; keine automatische Dateisystemformatierung,
  keine OTA-Schnittstelle mit Standardkennwort.
- Backup-Format auf `FeedMate-v1.24` aktualisiert.
- CORS von pauschalem `*` auf Same-Origin sowie Fleet-API-Aufrufe mit
  Gruppenschlüssel beschränkt; unbekannte Cross-Origin-Preflights abgewiesen.
- Scheduler führt beim Hochfahren keine bereits verstrichene Sekunde der
  aktuellen Minute nachträglich aus.
- Fremdbibliothekswarnung beim ESP8266-Build gezielt unterdrückt; der aktuelle
  vollständige Build läuft ohne Warnung durch.

## Automatisierte Prüfungen

| Prüfung | Ergebnis |
|---|---|
| 82 C++-Assertions: Motor, zeitbasierte Kalibrierung mit Hall-Sensor, Reservierungsidentität, Jam, Sensor, Portion, Auth-Session und Grenzwerte | bestanden |
| JavaScript-Syntax von `data/index.html` | bestanden |
| JavaScript-Syntax von `demo.html` | bestanden |
| Vollständiger ESP8266-D1-mini-Clean-Build einschließlich Link und BIN | bestanden, 48.048 B RAM / 407.751 B Flash |
| ESP8266-D1-mini-LittleFS-Image mit v1.25-Oberfläche | bestanden |
| ESP32-C3-LittleFS-Image mit v1.25-Oberfläche | bestanden |
| Vollständiger ESP32-C3-Build | nicht abgeschlossen; Assembler bricht beim Ermitteln seines Pfades mit Windows-Fehler 5 ab |
| ESP32-C6/S2/S3 vollständiger Build für v1.25 | in dieser Sitzung nicht ausgeführt |

Der ESP32-C3-Build nutzt PlatformIO 8.0.0 und Arduino-ESP32 3.3.12. In der
abgeschotteten Prüfumgebung bricht der GCC-14-Assembler beim Ermitteln seines
eigenen Pfades mit Windows-Fehler 5 ab. Das ist ein Werkzeug-/Umgebungsfehler,
kein gemeldeter C++-Compilerfehler. Der identische Werkzeugstand hat v1.23
außerhalb dieser Umgebung bereits gebaut, gelinkt und hochgeladen; das ersetzt
keinen vollständigen v1.25-Build.

Beim abschließenden ESP8266-Clean-Build wurde zunächst eine nicht portable
Header-Signatur entdeckt (`int` statt `size_t`); der Aufruf ist nun auf die
gemeinsame `size_t`-Schnittstelle festgelegt und der vollständige Build danach
erfolgreich wiederholt worden.

## Verbleibende Hardwaregrenzen

- Der Nutzer hat den AP nach dem Flashen mit mehreren Geräten gesucht und auch
  eine manuelle Verbindung versucht; kein Gerät findet oder erreicht ihn. Die
  serielle Meldung `active` basiert nur auf Treiber-, IP- und SSID-Abfragen.
  Deshalb ist die Funkfunktion offen und der AP-Status kein erfolgreicher
  Hardwaretest. Boardmodell, Antennenausführung und Funkmessung sind noch zu
  prüfen.
- Motorstrom, MOSFET-Pegel, Hall-Polarität, Lichtschranke und ADC-Teiler können
  ohne die konkrete Verdrahtung nicht elektrisch geprüft werden. Vor einem
  Motoranschluss muss die Pinbelegung aus `BOARDS.md` verifiziert werden.
- Das Verwaltungs-AP bleibt auf ausdrücklichen Wunsch dauerhaft aktiv. Das
  feste AP-Kennwort ist bequem, aber weniger sicher als ein individuelles
  Kennwort; das Admin-Kennwort muss nach der Ersteinrichtung geändert werden.
# v1.30 Zusatzprüfung Konfigurationsspeicherung

- UI sendet bei Zeitzonen-/Hostnameänderung nur geänderte Felder.
- Server übernimmt nicht gesendete Felder aus der vorhandenen Konfiguration.
- MQTT-/Portionsprüfungen sind an Änderungen des jeweiligen Blocks gebunden.
- JavaScript-Syntax und Live-Hardware-POST sind in diesem Schritt nicht ausgeführt.

