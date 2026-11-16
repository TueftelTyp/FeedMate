# FeedMate v1.24 installieren

## Zugangsdaten

- Verwaltungs-WLAN: `FeedMate-<MAC-Adresse>`
- WLAN-Kennwort: `feedmate`
- Adresse: `http://192.168.4.1`
- Erstes Admin-Kennwort: `FeedMateAdmin`
- Demo-Kennwort: `admin`

Beim geprüften Gerät mit der von esptool angezeigten MAC-Adresse
`44:bd:8d:5e:19:e8` lautet die SSID daher `FeedMate-44bd8d5e19e8`.

## ESP32-C3 vollständig neu installieren

Im entpackten Projektordner ausführen; `COM6` bei Bedarf ersetzen:

```powershell
pio run -e esp32c3 -t clean
pio run -e esp32c3 -t erase --upload-port COM6
pio run -e esp32c3 -t upload --upload-port COM6
pio run -e esp32c3 -t uploadfs --upload-port COM6
pio device monitor -b 115200 --port COM6
```

Nach dem Firmware-Upload kann Windows der nativen USB-Schnittstelle eine neue
COM-Nummer geben. In diesem Fall den neu erschienenen Port verwenden.

Im Monitor muss eine Zeile in dieser Form erscheinen:

```text
Management AP started: FeedMate-44bd8d5e19e8 Password: feedmate Channel: 1 IP: 192.168.4.1 mode=2
```

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
