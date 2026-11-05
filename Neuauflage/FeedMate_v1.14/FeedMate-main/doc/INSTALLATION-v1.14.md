# FeedMate v1.14 installieren

Diese Anleitung gilt für den klassischen **ESP32 DevKit / ESP32-WROOM-32** und den **WeMos D1 mini / ESP8266**. Der ESP32 ist das empfohlene Standardziel. ESP32-C3, S2, S3, C6 und andere Pinbelegungen benötigen Anpassungen.

## 1. Benötigt

- FeedMate-v1.14-Projektordner vollständig entpackt
- USB-Datenkabel; reine Ladekabel funktionieren nicht
- Visual Studio Code mit der Erweiterung **PlatformIO IDE** oder PlatformIO Core 6.x
- passender USB-Treiber für den verwendeten CP210x-/CH340-Adapter
- ESP32 DevKit/WROOM oder D1 mini/ESP8266 mit 4 MB Flash
- DS3231, Motor-MOSFET mit Freilaufbeschaltung und die vorgesehenen Sensoren

Vor dem ersten Flashen die Motorversorgung trennen. ESP und Motor dürfen keine unkontrollierte gemeinsame Versorgung verwenden. Alle Signale arbeiten mit 3,3-V-Logik.

## 2. Projekt öffnen

In Visual Studio Code **Datei → Ordner öffnen** wählen und den Ordner öffnen, in dem `platformio.ini` liegt. PlatformIO installiert die festgelegten Bibliotheken beim ersten Build automatisch.

Kommandozeile:

```sh
cd FeedMate-main
pio run -e esp32dev
```

Für ESP8266 stattdessen:

```sh
pio run -e d1_mini
```

Unter Windows sollte der Projektpfad kurz sein, zum Beispiel `C:\FeedMate`. Bei sehr langen Pfaden kann der ältere ESP32-Compiler mit `CreateProcess` abbrechen.

## 3. Pinbelegung

| Funktion | ESP32 DevKit/WROOM | D1 mini / ESP8266 |
|---|---:|---:|
| Motor-MOSFET Gate | GPIO25 | D5 / GPIO14 |
| Jam-Eingang, HIGH = Fehler | GPIO26 | D6 / GPIO12 |
| Lichtschranke | GPIO27 | D7 / GPIO13 |
| Hall-Sensor | GPIO33 | D8 / GPIO15 |
| manueller Taster gegen GND | GPIO32 | D3 / GPIO0 |
| DS3231 SDA | GPIO21 | D2 / GPIO4 |
| DS3231 SCL | GPIO22 | D1 / GPIO5 |
| Versorgungserkennung | GPIO34 | A0 |
| Status-LED | GPIO2 | LED_BUILTIN |

Beim ESP8266 müssen D8/GPIO15 beim Boot LOW und D3/GPIO0 HIGH bleiben. Details und elektrische Hinweise stehen in `doc/BOARDS.md`.

## 4. Seriellen Port ermitteln

Board per USB anschließen und in PlatformIO unter **Devices** den Port ablesen. Unter Windows ist das typischerweise `COM3`, unter Linux `/dev/ttyUSB0` oder `/dev/ttyACM0`.

Optional den Port in Befehlen ergänzen:

```sh
pio run -e esp32dev -t upload --upload-port COM3
```

## 5. Firmware und Weboberfläche erstmalig flashen

Beim ersten Einrichten werden sowohl Firmware als auch LittleFS-Weboberfläche benötigt.

### ESP32

```sh
pio run -e esp32dev
pio run -e esp32dev -t buildfs
pio run -e esp32dev -t upload --upload-port COM3
pio run -e esp32dev -t uploadfs --upload-port COM3
```

### ESP8266

```sh
pio run -e d1_mini
pio run -e d1_mini -t buildfs
pio run -e d1_mini -t upload --upload-port COM3
pio run -e d1_mini -t uploadfs --upload-port COM3
```

`uploadfs` ersetzt das komplette LittleFS-Dateisystem. Bei späteren Updates vorher ein Backup erstellen; WLAN-, MQTT-, Futter-, Zeitplan- und Logdaten können dabei verloren gehen. Für ein reines Firmwareupdate nur `upload` ausführen.

## 6. Erststart und Kennwort

Vor dem Neustart seriellen Monitor öffnen:

```sh
pio device monitor -b 115200 --port COM3
```

Dann Reset drücken. Beim ersten Start erscheinen im seriellen Monitor:

- das zufällig erzeugte Admin-Kennwort,
- ein Wiederherstellungscode für diesen Start,
- bei aktivem Fallback-WLAN dessen Name und zufälliges WLAN-Kennwort.

Das Erstkennwort sofort sicher speichern. Der Wiederherstellungscode wird bei jedem Neustart neu erzeugt und gilt nur für diesen Start.

## 7. WLAN über das Smartphone einrichten

Wenn FeedMate kein bekanntes WLAN erreicht, startet nach ungefähr 30 Sekunden ein eigenes Setup-WLAN. Mit dem im seriellen Monitor angezeigten Kennwort verbinden und im Smartphone-Browser öffnen:

`http://192.168.4.1`

Mit dem Admin-Kennwort anmelden. Unter **Einstellungen** mindestens konfigurieren:

- Gerätename
- WLAN-Name und WLAN-Kennwort
- Zeitzone, für Deutschland: `CET-1CEST,M3.5.0,M10.5.0/3`
- verwendete Sensoren
- optional MQTT und Home Assistant
- für mehrere Automaten denselben Gruppenschlüssel mit mindestens 16 Zeichen

Anschließend FeedMate neu starten. Die neue IP-Adresse im Router nachsehen oder im seriellen Monitor ablesen.

## 8. Motor und Sensoren ohne Futter prüfen

1. Motorversorgung zunächst getrennt lassen und prüfen, dass das Gate beim Boot nicht aktiv wird.
2. Jam-Eingang und Sensorpegel kontrollieren.
3. Motorversorgung anschließen und nur einen sehr kurzen Probelauf ausführen.
4. Jam-Erkennung während eines Laufs auslösen und die Abschaltung prüfen.
5. Hall-Sensor nur aktivieren, wenn tatsächlich Impulse erkannt werden.
6. RTC, lokale Uhrzeit und Statuslampe kontrollieren.

Die Software ersetzt keine Sicherung gegen einen elektrisch dauerhaft eingeschalteten Motor. Eine geeignete Sicherung, Freilaufbeschaltung und unabhängige Abschaltung sind erforderlich.

## 9. Futtersorte kalibrieren

Unter **Futter & Kalibrierung**:

1. Futtersorte benennen und Zeit- oder Umdrehungsmodus wählen.
2. Behälter unterstellen und Probelauf starten.
3. Ausgegebenes Futter extern wiegen.
4. Gramm und gegebenenfalls Hall-Umdrehungen eintragen.
5. Kalibrierung speichern.

FeedMate besitzt keine eingebaute Waage. Grammportionen sind proportionale Schätzungen aus dem gewogenen Probelauf.

## 10. Mehrere Automaten verbinden

Jeden Automaten zunächst einzeln installieren und denselben Gruppenschlüssel setzen. Danach auf dem verwaltenden Smartphone unter **Automaten** Name, lokale URL und Gruppenschlüssel hinzufügen. Im gemeinsamen Zeitplan können mehrere Automaten pro Fütterungszeit markiert werden.

Die Geräte müssen im selben erreichbaren lokalen Netz liegen. HTTP und MQTT sind unverschlüsselt; keine Portfreigabe ins Internet einrichten.

## 11. Demo verwenden

`FeedMate-Demo-v1.14.html` kann ohne ESP direkt im Browser geöffnet werden. Kennwort: `admin`. Nur in der Demo dürfen nicht existente Automaten hinzugefügt werden; Status, Fütterung und Zeitplanübertragung werden lokal simuliert. Diese Demo-Funktion ist nicht Bestandteil der produktiven `data/index.html`.

## 12. Häufige Probleme

- **Kein Port sichtbar:** anderes USB-Datenkabel oder CP210x-/CH340-Treiber verwenden.
- **Upload wartet auf Verbindung:** Boot-Taste gedrückt halten, kurz Reset drücken, Boot loslassen.
- **Weboberfläche fehlt:** passendes `uploadfs` für dasselbe Board ausführen.
- **Motor bleibt gesperrt:** Jam-Eingang, LittleFS-Status, Uhrzeit und Fehleranzeige prüfen.
- **Zeitplan läuft nicht:** Uhr muss gültig sein, Futtersorte kalibriert und Zeit aktiviert sein.
- **Passwort vergessen:** seriellen Monitor öffnen, neu starten und den neuen Recovery-Code in „Passwort vergessen?“ verwenden.
- **ESP32-Build meldet CreateProcess:** Projekt und PlatformIO-Core in einen kürzeren Pfad verschieben.

Vor dem unbeaufsichtigten Einsatz die Hardwaretests aus `doc/TEST_REPORT.md` durchführen.

