# FeedMate v1.23 installieren

## Zugangsdaten

- Setup-WLAN: `FeedMate-<Geräte-ID>`
- WLAN-Kennwort: `feedmate`
- Adresse: `http://192.168.4.1`
- Erstes Admin-Kennwort: `FeedMateAdmin`
- Demo-Kennwort: `admin`

Das Setup-WLAN bleibt auch neben einer erfolgreichen Heimnetz-Verbindung aktiv.
Eine Änderung des Gerätenamens ändert die AP-SSID nicht.

## ESP32-C3 installieren

Projekt entpacken, im Projektordner ein Terminal öffnen und ausführen:

```powershell
pio run -e esp32c3 -t clean
pio run -e esp32c3 -t erase --upload-port COM6
pio run -e esp32c3 -t upload --upload-port COM6
pio run -e esp32c3 -t uploadfs --upload-port COM6
```

`erase` löscht sämtliche Einstellungen. Den Port bei Bedarf anpassen. Nach dem
Schreiben das Board neu starten und mit dem FeedMate-WLAN verbinden.

## Weitere Ziele

- `esp32dev` – klassischer ESP32
- `esp32c3` – ESP32-C3
- `esp32c6` – ESP32-C6
- `esp32s2` – ESP32-S2
- `esp32s3` – ESP32-S3
- `d1_mini` – ESP8266 D1 mini

## Smartphone-Konfiguration

1. Mit dem FeedMate-AP verbinden und `http://192.168.4.1` öffnen.
2. Mit `FeedMateAdmin` anmelden.
3. Unter **Einstellungen** Heimnetz und Gerätename eintragen.
4. Die Zeitzone aus der Liste wählen, beispielsweise `Europe/Berlin`.
5. Speichern und den Automaten neu starten.
6. Das Admin-Kennwort durch ein eigenes Kennwort mit mindestens 16 Zeichen ersetzen.

Der Gerätename wird nach dem Neustart als bereinigter Hostname verwendet.
Leerzeichen und Unterstriche werden zu Bindestrichen. Die chipbezogene
Setup-SSID bleibt unverändert.

## Oberfläche mit übertragen

Nach einem Firmware-Upload auch das Dateisystem schreiben:

```powershell
pio run -e esp32c3 -t uploadfs --upload-port COM6
```

Ohne `uploadfs` kann noch die Oberfläche einer älteren Version erscheinen.

## Demo

`FeedMate-Demo-v1.23.html` im Browser öffnen und mit `admin` anmelden. Die Demo
steuert keine reale Hardware und speichert ihre Änderungen nur im Browser.
