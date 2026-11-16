# FeedMate v1.21 installieren

Diese Anleitung gilt für **ESP32 DevKit/WROOM, ESP32-C3, ESP32-C6, ESP32-S2, ESP32-S3** und **WeMos D1 mini/ESP8266**. Für Espressif-Referenzboards sind fertige Ziele enthalten. Bei SuperMini-, XIAO-, LOLIN- und anderen Platinenlayouts muss die Pinbelegung nach `doc/BOARDS.md` angepasst werden.

## 1. Benötigt

- FeedMate-v1.21-Projektordner vollständig entpackt
- USB-Datenkabel; reine Ladekabel funktionieren nicht
- Visual Studio Code mit der Erweiterung **PlatformIO IDE** oder PlatformIO Core 6.x
- passender USB-Treiber für den verwendeten CP210x-/CH340-Adapter
- ein unterstütztes ESP32-/ESP8266-Board mit ausreichend freien GPIOs
- DS3231, Motor-MOSFET mit Freilaufbeschaltung und die vorgesehenen Sensoren

Vor dem ersten Flashen die Motorversorgung trennen. ESP und Motor dürfen keine unkontrollierte gemeinsame Versorgung verwenden. Alle Signale arbeiten mit 3,3-V-Logik.

## 2. Projekt öffnen

In Visual Studio Code **Datei → Ordner öffnen** wählen und den Ordner öffnen, in dem `platformio.ini` liegt. PlatformIO installiert die festgelegten Bibliotheken beim ersten Build automatisch.

Kommandozeile für den klassischen ESP32:

```sh
cd FeedMate-main
pio run -e esp32dev
```

Andere fertige Ziele:

```sh
pio run -e esp32c3
pio run -e esp32c6
pio run -e esp32s2
pio run -e esp32s3
pio run -e d1_mini
```

Unter Windows sollte der Projektpfad kurz sein, zum Beispiel `C:\FeedMate`. Bei sehr langen Pfaden kann der ältere ESP32-Compiler mit `CreateProcess` abbrechen.

## 3. Pinbelegung

Die vollständige Tabelle für ESP32, C3, C6, S2, S3 und ESP8266 steht in `doc/BOARDS.md`. Vor dem Verdrahten unbedingt das konkrete Boardlayout prüfen. Die Motorleitung benötigt einen externen Pulldown; native USB-, Flash-/PSRAM- und Boot-Strapping-Pins dürfen nicht unkontrolliert belastet werden.

## 4. Seriellen Port ermitteln

Board per USB anschließen und in PlatformIO unter **Devices** den Port ablesen. Unter Windows ist das typischerweise `COM3`, unter Linux `/dev/ttyUSB0` oder `/dev/ttyACM0`.

Optional den Port in Befehlen ergänzen:

```sh
pio run -e esp32c3 -t upload --upload-port COM6
```

## 5. Firmware und Weboberfläche erstmalig flashen

Beim ersten Einrichten werden sowohl Firmware als auch LittleFS-Weboberfläche benötigt.

### Gewähltes Ziel

```sh
pio run -e esp32c3
pio run -e esp32c3 -t buildfs
pio run -e esp32c3 -t upload --upload-port COM6
pio run -e esp32c3 -t uploadfs --upload-port COM6
```

`esp32c3` und `COM6` durch das tatsächlich verwendete Ziel und den ermittelten Port ersetzen. Firmware und LittleFS immer mit demselben Ziel erzeugen.

Die ESP32-Ziele verwenden das 4-MB-Schema `huge_app.csv`. Es stellt 3 MB für die Firmware und 896 KiB für LittleFS bereit. Das Schema besitzt keine zweite OTA-Partition; Aktualisierungen erfolgen bei FeedMate weiterhin per USB/serieller Schnittstelle. Beim Wechsel von einem älteren Partitionsschema Firmware und LittleFS neu flashen und vorher ein Backup exportieren.

### ESP8266

```sh
pio run -e d1_mini
pio run -e d1_mini -t buildfs
pio run -e d1_mini -t upload --upload-port COM3
pio run -e d1_mini -t uploadfs --upload-port COM3
```

`uploadfs` ersetzt das komplette LittleFS-Dateisystem. Bei späteren Updates vorher ein Backup erstellen; WLAN-, MQTT-, Futter-, Zeitplan- und Logdaten können dabei verloren gehen. Für ein reines Firmwareupdate nur `upload` ausführen.

## 6. Erststart und Kennwort

Für die Ersteinrichtung ist kein serieller Monitor erforderlich:

- Kennwort des FeedMate-Setup-WLANs: `feedmate`
- initiales Kennwort der Verwaltungsoberfläche: `FeedMateAdmin`

Nach der ersten Anmeldung unter **Einstellungen → System & Gruppe** ein persönliches Admin-Kennwort mit mindestens 16 Zeichen setzen. Der feste Erstzugang ist bequem, aber öffentlich bekannt.

Optional kann der serielle Monitor mit `pio device monitor -b 115200 --port COM3` geöffnet werden. Dort erscheint bei jedem Start weiterhin ein zufälliger, nur bis zum nächsten Neustart gültiger Wiederherstellungscode für den Notfall.

Bei C3/C6/S2/S3 ist die native USB-Konsole ab v1.21 im Hardware-CDC-Modus beim Start aktiviert. Windows kann ihr nach dem Flashen eine neue COM-Nummer geben; den Port dann in PlatformIO erneut ablesen. Ein erfolgreicher AP-Start erscheint als `Setup AP started: FeedMate-... Password: feedmate IP: 192.168.4.1`. Bei `Setup AP FAILED` versucht FeedMate den Start automatisch alle fünf Sekunden erneut. Solange das Setup-AP aktiv ist, werden SSID, IP und Anzahl verbundener Clients alle 30 Sekunden erneut ausgegeben.

## 7. WLAN über das Smartphone einrichten

Wenn FeedMate kein bekanntes WLAN erreicht, startet nach ungefähr 30 Sekunden ein eigenes Setup-WLAN namens `FeedMate-...`. Mit dem WLAN-Kennwort `feedmate` verbinden und im Smartphone-Browser öffnen:

`http://192.168.4.1`

Mit `FeedMateAdmin` anmelden. Unter **Einstellungen** mindestens konfigurieren:

- Gerätename
- WLAN-Name und WLAN-Kennwort
- Zeitzone, für Deutschland: `CET-1CEST,M3.5.0,M10.5.0/3`
- verwendete Sensoren

Bei einem reinen Firmwareupdate bleibt ein bereits gespeichertes Admin-Kennwort erhalten. Soll ein schon initialisiertes Gerät wieder den festen Erstzugang erhalten, vorher ein Backup erstellen und anschließend auch LittleFS neu übertragen; dadurch werden die vorhandenen Einstellungen gelöscht.
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

`FeedMate-Demo-v1.21.html` kann ohne ESP direkt im Browser geöffnet werden. Kennwort: `admin`. Nur in der Demo dürfen nicht existente Automaten hinzugefügt werden; Status, Fütterung und Zeitplanübertragung werden lokal simuliert. Diese Demo-Funktion ist nicht Bestandteil der produktiven `data/index.html`.

## 12. Häufige Probleme

- **Kein Port sichtbar:** anderes USB-Datenkabel oder CP210x-/CH340-Treiber verwenden.
- **Upload wartet auf Verbindung:** Boot-Taste gedrückt halten, kurz Reset drücken, Boot loslassen.
- **Weboberfläche fehlt:** passendes `uploadfs` für dasselbe Board ausführen.
- **Motor bleibt gesperrt:** Jam-Eingang, LittleFS-Status, Uhrzeit und Fehleranzeige prüfen.
- **Zeitplan läuft nicht:** Uhr muss gültig sein, Futtersorte kalibriert und Zeit aktiviert sein.
- **Passwort vergessen:** seriellen Monitor öffnen, neu starten und den neuen Recovery-Code in „Passwort vergessen?“ verwenden.
- **ESP32-Build meldet CreateProcess:** Projekt und PlatformIO-Core in einen kürzeren Pfad verschieben.

Vor dem unbeaufsichtigten Einsatz die Hardwaretests aus `doc/TEST_REPORT.md` durchführen.

