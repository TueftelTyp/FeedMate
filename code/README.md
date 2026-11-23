# FeedMate – ESP32-/ESP8266-Firmware

## v1.34: Gruppenschlüssel ab 4 Zeichen

Bei Änderungen an WLAN, Hostname oder Zeitzone fragt die Oberfläche, ob nach
dem Speichern neu gestartet werden soll. Andere Einstellungen benötigen keinen
Neustart. Admin-Passwörter haben mindestens 10, Gruppenschlüssel mindestens 4
Zeichen.

## v1.32: Gruppenschlüssel mit vier Zeichen

PlatformIO baut standardmäßig das Profil `esp32c3_supermini`. Für ESP32 wird
die AP-Sendeleistung auf einen konservativen, expliziten Wert gesetzt und ein
Fehler dabei seriell gemeldet. Das bestätigt weiterhin nicht den RF-Empfang.

Das Formular überträgt nur geänderte Einstellungen. Die Firmware validiert MQTT-, Zeitzonen- und Portionswerte nur, wenn der jeweilige Block geändert wurde.

Der manuelle Taster quittiert einen Motorfehler per kurzem Druck, sofern der Jam-Eingang nicht mehr aktiv ist. Bei weiterhin aktivem Fehler bleibt die Quittierung gesperrt.

Gruppenschlüssel akzeptieren jetzt mindestens 4 Zeichen. Kürzere Schlüssel sind einfacher zu teilen, aber leichter zu erraten; nur in einem vertrauenswürdigen lokalen Netz verwenden.

Änderungen an WLAN, Hostname oder Zeitzone fragen beim Speichern, ob sofort neu gestartet werden soll. Bei „Nein“ werden die Werte gespeichert und beim späteren Neustart angewendet.

## v1.31: Admin-Passwort und Motorfehler per Taste

Der ESP32-Hostname wird vor jedem `WiFi.mode()` gesetzt, wie es die Arduino-
ESP32-API verlangt. Ohne gespeichertes Heimnetz startet das Gerät im reinen
AP-Modus; AP+STA wird nur mit konfigurierten Router-Zugangsdaten verwendet.
Die Routerverbindung wartet weiterhin, bis Setup-Clients getrennt sind.

## v1.26: stabiles Setup-WLAN beim Heimnetzstart

Das Verwaltungs-AP wird ohne nachträgliches Ausschalten des WLANs gestartet.
Hostname und WLAN-Modus werden vor dem AP-Start gesetzt; das gespeicherte
Heimnetz wird erst verbunden, wenn kein Smartphone mehr am Setup-AP angemeldet
ist. Die AP-Überwachung startet einen bereits intakten Zugangspunkt nicht erneut.

## v1.25: zuverlässigere Kalibrierung und Gruppenzeitpläne

Probeläufe sind jetzt tatsächlich laufzeitgesteuert, auch wenn ein Hall-Sensor
aktiviert ist; der Hall-Sensor bleibt währenddessen für Fehler und Grenzwerte
überwacht. Ein Probelauf meldet Abbrüche statt sie als Erfolg darzustellen.
Gruppenzeitpläne übertragen auch leere Zeitlisten, sodass entfernte Automaten
alte Fütterungen löschen. Reservierungen bleiben beim Sortieren an der gleichen
Fütterungszeit erhalten. Die Übersicht zeigt den AP-Treiberstatus und Clients;
das ist ausdrücklich kein Nachweis einer tatsächlichen Funkabstrahlung.

## v1.24: korrigierte AP-SSID und Selbstüberwachung

Die Geräte-ID wird nun in derselben Byte-Reihenfolge wie die von esptool
ausgegebene MAC-Adresse dargestellt. Das dauerhaft aktive Verwaltungs-WLAN
wird alle zwei Sekunden geprüft und bei Verlust von Modus, IP oder SSID neu
gestartet. Die Übersicht zeigt seinen Zustand direkt an. ESP32-C3 und C6 geben
die Diagnose über die native USB-Konsole aus. Browserzugriffe von anderen
Automaten verwenden nur noch die CORS-Freigabe mit gültigem Gruppenschlüssel.

## v1.23: Zeitzonenauswahl und dauerhaftes Verwaltungs-AP

Die Zeitzone wird in der Smartphone-Oberfläche aus verständlichen Namen wie
`Europe/Berlin` gewählt. FeedMate übersetzt sie intern in die benötigte
POSIX-Regel. Das Verwaltungs-AP bleibt nun auch nach erfolgreicher Verbindung
mit dem Heimnetz aktiv. Der Gerätename wird als Netzwerk-Hostname verwendet,
ändert aber nicht die stabile AP-SSID `FeedMate-<Geräte-ID>`.

## v1.22: Setup-AP startet vor den Netzwerkdiensten

Das Setup-WLAN verwendet beim Erststart das feste Kennwort `feedmate`; die Verwaltungsoberfläche verwendet initial `FeedMateAdmin`. Damit lässt sich FeedMate vollständig per Smartphone einrichten. Das Admin-Kennwort sollte nach der ersten Anmeldung in den Einstellungen durch ein persönliches Kennwort mit mindestens 10 Zeichen ersetzt werden. Der serielle Einmal-Wiederherstellungscode bleibt als Notfallweg erhalten.

Das Setup-AP startet nach dem Laden der Konfiguration und vor RTC, Webserver und MQTT. Ein Fehler in diesen Netzwerkdiensten verhindert das Smartphone-WLAN nicht. Der AP-Start wird geprüft und bei einem Fehlschlag erneut versucht.

## v1.15: Bereinigtes Release-Paket

Das vollständige Release-ZIP enthält nur noch Dateien, die für die aktuelle Version relevant sind. Historische Quellcodekopien, frühere versionsgebundene Installationsanleitungen und alte einzelne Versionsnotizen wurden aus dem Release-Paket entfernt. Der aktuelle Quellcode, Dokumentation, Prüfprogramme und die Unterstützung für ESP32 und ESP8266 bleiben vollständig enthalten.

## Aktuelle Funktionen

- Smartphone-optimierte, kennwortgeschützte Verwaltung mit Statusanzeige, Zeitplan, Kalibrierung, Automaten, Backup und verständlichem Verlauf.
- Sekundengenaue Zeitpläne mit bis zu 32 Einträgen je Automat und gemeinsamer Verteilung an mehrere ausgewählte Automaten.
- Wiederverwendbare Futterprofile: Probelauf durchführen, Futter extern wiegen und Portionen anschließend aus Laufzeit oder Hall-Umdrehungen berechnen.
- ESP32, ESP32-C3/C6, ESP32-S2/S3 und ESP8266 über dieselbe Codebasis.
- Schutz vor doppelten Fütterungen nach Neustart, gemeinsame Motor-Zustandsverwaltung, RTC-/Netzwerkzeitsynchronisation, MQTT und dauerhafte Logs.
- Eigenständige [Demo](demo.html) mit Kennwort **admin**; simulierte Automaten existieren ausschließlich in der Demo.
- Fester Erstzugang für die Smartphone-Einrichtung: Setup-WLAN **feedmate**, Verwaltung **FeedMateAdmin**.
- Geprüfter Setup-AP-Start vor RTC und Webserver; Heimnetzverbindung wartet auf getrennte Setup-Clients.
- Vollständige [Installationsanleitung v1.27](doc/INSTALLATION-v1.27.md).

FeedMate steuert einen Futterautomaten auf **ESP32, ESP32-C3/C6, ESP32-S2/S3** oder **WeMos D1 mini / ESP8266** mit DS3231, Motor-MOSFET und optionalen Sensoren. Diese Firmware verwendet Arduino-C++ und PlatformIO, kein ESPHome.

## Einstieg

1. [Installation und Migration](doc/HowToFlash.md) lesen; bestehende Flash-Daten sichern.
2. In [Board und Pinbelegung](doc/BOARDS.md) das passende Ziel auswählen, beispielsweise `esp32c3`, `esp32c6`, `esp32s2`, `esp32s3`, `esp32dev` oder `d1_mini`.
3. Firmware und bei der Ersteinrichtung das LittleFS-Image per USB übertragen.
4. Mit dem Setup-WLAN und Kennwort **feedmate** verbinden. Ein serieller Monitor ist für die Ersteinrichtung nicht erforderlich; dort erscheint weiterhin der Notfall-Wiederherstellungscode.
5. Unter http://192.168.4.1 mit **FeedMateAdmin** anmelden, ein persönliches Admin-Kennwort setzen, WLAN und Zeitzone konfigurieren und neu starten.
6. Erst nach Prüfung der Motor- und Sensorfunktion Zeitpläne aktivieren. Neue und alte unmarkierte Zeitpläne sind zunächst deaktiviert.

## Verhalten

- Bis zu 32 Zeitplaneinträge je Automat und verständliche Zeitzonenauswahl, etwa `Europe/Berlin`.
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
| doc/INSTALLATION-v1.27.md | Vollständige aktuelle Installationsanleitung |

## Sicherheit und Betrieb

Admin-Anmeldung verwendet einen gerätespezifisch gesalzenen SHA-256-Hash, zufällige 256-Bit-Sitzungstokens, 30 Minuten absolute Gültigkeit und eine Anmelderate von höchstens einem Versuch pro zwei Sekunden. Das feste Erstkennwort `FeedMateAdmin` muss nach der Einrichtung durch ein individuelles Kennwort mit mindestens 10 Zeichen ersetzt werden. Der Hash ist kein langsamer Passwort-KDF; schwache selbst gewählte Passwörter vermeiden. Bei vergessenem Passwort den **USB/Serial-Wiederherstellungscode** aus dem aktuellen Start in der Smartphone-Oberfläche eingeben. Er ist einmalig, auf fünf Fehlversuche pro Start begrenzt und wird nicht dauerhaft gespeichert.

HTTP und MQTT sind in diesem Stand **unverschlüsselt**. Nur in einem vertrauenswürdigen, abgeschotteten lokalen Netz betreiben; keine Internet-Portfreigabe. MQTT-Zugang über Broker-Anmeldung und Topic-ACLs schützen. API-Tokens werden nur im Arbeitsspeicher des Browsers gehalten. Cross-Origin-Zugriff ist für die Gruppenverwaltung nur mit dem gültigen Gruppenschlüssel freigegeben. Der bisherige OTA-Endpunkt mit festem Passwort wurde entfernt; Updates erfolgen per USB.

Ein Softwaretest bestätigt weder die tatsächliche Futtermenge noch die elektrische Abschaltung. Hall-Impulse bestätigen Bewegung, die Lichtschranke nur Futter vorhanden/fehlt. Eine unabhängige Motorabschaltung ist für harte Abschaltgarantien erforderlich. Beim ESP8266: GPIO15/D8 ist ein Boot-Strapping-Pin und muss beim Boot LOW bleiben; GPIO0/D3 darf beim Boot nicht gedrückt sein. Die ursprünglichen Hardwaredokumente sind nicht elektrisch validiert. Insbesondere RTC-Pufferung und Versorgung separat prüfen.

Deep Sleep wird nicht automatisch ausgelöst. SMTP-/Messenger-Benachrichtigungen, automatische Updates, Grammkalibrierung und zeitbasierte Log-Aufbewahrung sind nicht implementiert. Alte Konfigurationsfelder dazu sind keine Funktionszusagen.

## Tests wiederholen

Linux/macOS mit C++17-Compiler:

```sh
g++ -std=c++17 tests/controller_test.cpp -o /tmp/feedmate-tests
TZ=UTC /tmp/feedmate-tests
pio run -e esp32dev -e esp32c3 -e esp32c6 -e esp32s2 -e esp32s3 -e d1_mini
pio run -e esp32dev -e esp32c3 -e esp32c6 -e esp32s2 -e esp32s3 -e d1_mini -t buildfs
```

Windows: Zig 0.13.0 auf PATH, dann `tests\run-windows.cmd` aus dem Projektverzeichnis. GitHub Actions enthält dieselben Firmware-/Dateisystem-Builds sowie den C++-Test; der Workflow wurde hinzugefügt, aber nicht auf GitHub ausgeführt.

Lizenz: bestehende [LICENSE](LICENSE), CC BY-NC-SA 4.0. Ursprung: TueftelTyp/FeedMate. Überarbeitung vom 12.11.2026.
