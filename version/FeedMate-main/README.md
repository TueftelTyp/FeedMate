# FeedMate – ESP32-/ESP8266-Firmware

## v1.15: Bereinigtes Release-Paket

Das vollständige Release-ZIP enthält nur noch Dateien, die für die aktuelle Version relevant sind. Historische Quellcodekopien, frühere versionsgebundene Installationsanleitungen und alte einzelne Versionsnotizen wurden aus dem Release-Paket entfernt. Der aktuelle Quellcode, Dokumentation, Prüfprogramme und die Unterstützung für ESP32 und ESP8266 bleiben vollständig enthalten.

## Aktuelle Funktionen

- Smartphone-optimierte, kennwortgeschützte Verwaltung mit Statusanzeige, Zeitplan, Kalibrierung, Automaten, Backup und verständlichem Verlauf.
- Sekundengenaue Zeitpläne mit bis zu 32 Einträgen je Automat und gemeinsamer Verteilung an mehrere ausgewählte Automaten.
- Wiederverwendbare Futterprofile: Probelauf durchführen, Futter extern wiegen und Portionen anschließend aus Laufzeit oder Hall-Umdrehungen berechnen.
- ESP32 als empfohlene Plattform sowie ESP8266-Kompatibilität über dieselbe Codebasis.
- Schutz vor doppelten Fütterungen nach Neustart, gemeinsame Motor-Zustandsverwaltung, RTC-/Netzwerkzeitsynchronisation, MQTT und dauerhafte Logs.
- Eigenständige [Demo](demo.html) mit Kennwort **admin**; simulierte Automaten existieren ausschließlich in der Demo.
- Vollständige [Installationsanleitung v1.15](doc/INSTALLATION-v1.15.md).

FeedMate steuert einen Futterautomaten auf einem **ESP32 DevKit / ESP32-WROOM** (Standard) oder **WeMos D1 mini / ESP8266** mit DS3231, Motor-MOSFET und optionalen Sensoren. Diese Firmware verwendet Arduino-C++ und PlatformIO, kein ESPHome.

## Einstieg

1. [Installation und Migration](doc/HowToFlash.md) lesen; bestehende Flash-Daten sichern.
2. [Board und Pinbelegung](doc/BOARDS.md) auswählen. Mit PlatformIO `pio run -e esp32dev` und `pio run -e esp32dev -t buildfs` ausführen; für ESP8266 jeweils `-e d1_mini`.
3. Firmware und bei der Ersteinrichtung das LittleFS-Image per USB übertragen.
4. Seriellen Monitor mit 115200 Baud öffnen. Ein neues Admin-Passwort erscheint **einmalig**; bei jedem Start erscheint ein neuer Wiederherstellungscode. Das zufällige Setup-WLAN-Passwort erscheint bei jedem Start des Fallback-AP.
5. Im Setup-WLAN die Adresse http://192.168.4.1 öffnen, anmelden, WLAN und Zeitzone konfigurieren und neu starten.
6. Erst nach Prüfung der Motor- und Sensorfunktion Zeitpläne aktivieren. Neue und alte unmarkierte Zeitpläne sind zunächst deaktiviert.

## Verhalten

- Bis zu 32 Zeitplaneinträge je Automat, lokale Uhrzeit nach POSIX-Zeitzonenregel. Vorgabe UTC; Deutschland: `CET-1CEST,M3.5.0,M10.5.0/3`.
- Eine geplante Fütterung wird vor Motorstart dauerhaft für ihr Kalenderdatum reserviert. Neustarts, Uhr-Rücksprünge und Änderungen desselben Slots setzen diese Reservierung nicht zurück.
- Reservierung bedeutet **höchstens einen Versuch pro Slot und Datum**, keine garantierte Futterausgabe. Ein Stromausfall zwischen Reservierung und Motorstart kann eine Portion auslassen. Es gibt keine automatische Wiederholung oder Nachholung verpasster Minuten.
- Gleichzeitige Slots werden nacheinander abgearbeitet, soweit ihre geplante Minute noch läuft. Netzwerk und Webstatus pausieren während eines Motorlaufs (maximal 30 Sekunden).
- Die gemeinsame Motorsteuerung schützt alle Eingänge: Web, MQTT, Zeitplan und Taster. Eine Fehlerquittierung startet niemals automatisch den Motor.
- Ohne gültige Uhrzeit bleiben Zeitpläne gesperrt; manuelle Fütterung ist bei gesundem Speicher und Sensorstatus weiterhin möglich.
- Die RTC speichert UTC. Beim Boot wird nur eine plausible RTC ohne Power-Loss-Flag übernommen; Netzwerkzeit korrigiert die Systemuhr und anschließend die RTC. Kein Setzen auf Kompilierzeit.
- Log-Zeitstempel sind UTC-Epochsekunden, bei unbekannter Uhrzeit 0. `uptime` enthält zusätzlich Millisekunden seit Boot.

## Projektstruktur

| Pfad | Inhalt |
|---|---|
| code/platform.h | Gemeinsame Plattformanbindung und getrennte Pinbelegungen |
| code/ | Aktive Firmware mit gemeinsamen Schnittstellen |
| data/index.html | Aktuelle, zur API passende lokale Bedienoberfläche |
| tests/ | Tests der tatsächlichen Motor- und Zeitplanimplementierung mit simulierten Ein-/Ausgängen |
| doc/CHANGES.md | Änderungen, API- und MQTT-Vertrag, bekannte Grenzen |
| doc/TEST_REPORT.md | Ausgeführte Prüfungen und verbleibende Hardwaretests |
| doc/INSTALLATION-v1.15.md | Vollständige aktuelle Installationsanleitung |

## Sicherheit und Betrieb

Admin-Anmeldung verwendet einen gerätespezifisch gesalzenen SHA-256-Hash, zufällige 256-Bit-Sitzungstokens, 30 Minuten absolute Gültigkeit und eine Anmelderate von höchstens einem Versuch pro zwei Sekunden. Das generierte Erstpasswort besitzt 128 Bit Zufallsentropie. Für Änderungen mindestens 16 Zeichen verwenden, vorzugsweise ein zufällig generiertes Passwort. Der Hash ist kein langsamer Passwort-KDF; schwache selbst gewählte Passwörter vermeiden. Bei vergessenem Passwort den **USB/Serial-Wiederherstellungscode** aus dem aktuellen Start in der Smartphone-Oberfläche eingeben. Er ist einmalig, auf fünf Fehlversuche pro Start begrenzt und wird nicht dauerhaft gespeichert.

HTTP und MQTT sind in diesem Stand **unverschlüsselt**. Nur in einem vertrauenswürdigen, abgeschotteten lokalen Netz betreiben; keine Internet-Portfreigabe. MQTT-Zugang über Broker-Anmeldung und Topic-ACLs schützen. API-Tokens werden nur im Arbeitsspeicher des Browsers gehalten. Der bisherige OTA-Endpunkt mit festem Passwort wurde entfernt; Updates erfolgen per USB.

Ein Softwaretest bestätigt weder die tatsächliche Futtermenge noch die elektrische Abschaltung. Hall-Impulse bestätigen Bewegung, die Lichtschranke nur Futter vorhanden/fehlt. Eine unabhängige Motorabschaltung ist für harte Abschaltgarantien erforderlich. Beim ESP8266: GPIO15/D8 ist ein Boot-Strapping-Pin und muss beim Boot LOW bleiben; GPIO0/D3 darf beim Boot nicht gedrückt sein. Die ursprünglichen Hardwaredokumente sind nicht elektrisch validiert. Insbesondere RTC-Pufferung und Versorgung separat prüfen.

Deep Sleep wird nicht automatisch ausgelöst. SMTP-/Messenger-Benachrichtigungen, automatische Updates, Grammkalibrierung und zeitbasierte Log-Aufbewahrung sind nicht implementiert. Alte Konfigurationsfelder dazu sind keine Funktionszusagen.

## Tests wiederholen

Linux/macOS mit C++17-Compiler:

```sh
g++ -std=c++17 tests/controller_test.cpp -o /tmp/feedmate-tests
TZ=UTC /tmp/feedmate-tests
pio run -e esp32dev -e d1_mini
pio run -e esp32dev -e d1_mini -t buildfs
```

Windows: Zig 0.13.0 auf PATH, dann `tests\run-windows.cmd` aus dem Projektverzeichnis. GitHub Actions enthält dieselben Firmware-/Dateisystem-Builds sowie den C++-Test; der Workflow wurde hinzugefügt, aber nicht auf GitHub ausgeführt.

Lizenz: bestehende [LICENSE](LICENSE), CC BY-NC-SA 4.0. Ursprung: TueftelTyp/FeedMate. Überarbeitung vom 05.11.2026.
