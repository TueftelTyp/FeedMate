# FeedMate v1.35 installieren

## Zugangsdaten

- Verwaltungs-WLAN: `FeedMate-<MAC-Adresse>`
- WLAN-Kennwort: `feedmate`
- Adresse: `http://192.168.4.1`
- Erstes Admin-Kennwort: `FeedMateAdmin`
- Demo-Kennwort: `admin`

Beim geprüften Gerät mit der von esptool angezeigten MAC-Adresse
`44:bd:8d:5e:19:e8` lautet die SSID daher `FeedMate-44bd8d5e19e8`.

## Komplettflash mit einem Befehl

Im Projektordner PowerShell öffnen und ausführen; Port bei Bedarf anpassen:

```powershell
.\scripts\flash-c3-supermini.ps1 -Port COM6
```

Das Skript stoppt bei jedem Fehler. Es löscht auch LittleFS mit Einstellungen,
Zeitplan und Logs.

Alternativ alle Befehle einzeln ausführen:

```powershell
pio run -e esp32c3_supermini -t clean
pio run -e esp32c3_supermini -t erase --upload-port COM6
pio run -e esp32c3_supermini -t upload --upload-port COM6
pio run -e esp32c3_supermini -t uploadfs --upload-port COM6
```

Nach erfolgreichem Upload kannst du die serielle Ausgabe so öffnen:

```powershell
pio device monitor -b 115200 --port COM6
```

Nach dem Firmware-Upload kann Windows der nativen USB-Schnittstelle eine neue
COM-Nummer geben. In diesem Fall den neu erschienenen Port verwenden.

Das Setup-WLAN startet nach dem Laden der Konfiguration und vor RTC/Webserver. Im Monitor sollte eine Zeile in dieser Form erscheinen:

```text
Management AP started: FeedMate-44bd8d5e19e8 Password: feedmate Channel: 1 IP: 192.168.4.1 mode=2
```

Das Heim-WLAN wird erst verbunden, wenn kein Smartphone mehr mit dem Setup-AP verbunden ist. Damit bleibt der Einrichtungszugang während der Bedienung stabil.

Anschließend das Smartphone mit dieser SSID verbinden, eine Warnung über ein
Netz ohne Internet bestätigen und `http://192.168.4.1` ausdrücklich im Browser
öffnen. Mit `FeedMateAdmin` anmelden und das Admin-Kennwort danach ändern.

## Andere Boards

Den Umgebungsnamen in den Befehlen ersetzen:

- `esp32dev` – klassischer ESP32
- `esp32c3` – ESP32-C3
- `esp32c6` – ESP32-C6
- `esp32s2` – ESP32-S2
- `esp32s3` – ESP32-S3
- `d1_mini` – ESP8266 D1 mini

Die Referenz-Pinbelegung steht in `doc/BOARDS.md`. Abweichende Boards dürfen
erst nach Anpassung und Prüfung der Pins mit Motorhardware verbunden werden.

## Oberfläche und Zeitzone

`uploadfs` ist erforderlich, weil die Weboberfläche in LittleFS liegt. In den
Einstellungen wird die Zeitzone aus der Liste gewählt, etwa `Europe/Berlin`.
Unbekannte Freitextwerte werden von Firmware und API abgelehnt.

## Demo

`demo.html` direkt im Browser öffnen und mit `admin` anmelden. Die Demo steuert
keine Hardware. Nur dort können erfundene Automaten hinzugefügt werden.

## Verwaltungs-WLAN-Verbindung bricht ab

v1.35 schaltet das WLAN nach dem AP-Start nicht mehr aus. Die Heimnetzverbindung beginnt erst, wenn das Smartphone den Setup-AP verlassen hat. Bleibt die Verbindung dennoch instabil, sichere im seriellen Monitor (115200 Baud) die Zeilen zu `Management AP`, `Starting saved Wi-Fi connection`, `Clients` und Neustarts.

## Verwaltungs-WLAN wird nicht gefunden

Die serielle Meldung `Management AP active` bestätigt nur, dass der WLAN-Treiber
AP-Modus, IP und SSID meldet. Sie bestätigt keine Funkabstrahlung. Wenn kein
Gerät die SSID sieht, teste das ESP32-C3-Board zunächst nur über USB, ohne
Motor/Motortreiber oder weitere externe Verdrahtung. Suche aus unmittelbarer
Nähe zusätzlich mit einem zweiten 2,4-GHz-fähigen Gerät. Bleibt der AP unsichtbar,
Boardmodell und Antennenausführung prüfen; erneutes Löschen und Flashen ist dann
kein aussagekräftiger Funknachweis.
