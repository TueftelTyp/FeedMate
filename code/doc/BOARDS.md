# Unterstützte Boards und Pinprofile

FeedMate verwendet eine gemeinsame Firmware für ESP32, ESP32-S2, ESP32-S3,
ESP32-C3, ESP32-C6 und ESP8266. Der klassische ESP32 bleibt das Standardziel.
Für jede moderne Chipfamilie ist ein direkt auswählbares Espressif-DevKit-Profil
enthalten. Andere Platinen derselben Chipfamilie werden über dasselbe Ziel mit
einer angepassten PlatformIO-Board-ID und Pinbelegung eingebunden.

## Fertige PlatformIO-Ziele

| Ziel | Referenzboard | Upload-Beispiel |
|---|---|---|
| `esp32dev` | ESP32 DevKit / WROOM | `pio run -e esp32dev -t upload` |
| `esp32c3` | ESP32-C3-DevKitM-1 | `pio run -e esp32c3 -t upload` |
| `esp32c6` | ESP32-C6-DevKitC-1 | `pio run -e esp32c6 -t upload` |
| `esp32s2` | ESP32-S2-Saola-1 | `pio run -e esp32s2 -t upload` |
| `esp32s3` | ESP32-S3-DevKitC-1 | `pio run -e esp32s3 -t upload` |
| `d1_mini` | WeMos D1 mini / ESP8266 | `pio run -e d1_mini -t upload` |

Den seriellen Port bei Bedarf ergänzen, zum Beispiel:

```sh
pio run -e esp32c3 -t upload --upload-port COM6
pio run -e esp32c3 -t buildfs
pio run -e esp32c3 -t uploadfs --upload-port COM6
```

Firmware und LittleFS müssen immer mit demselben Ziel gebaut werden. `uploadfs`
ersetzt die gespeicherte Konfiguration; vorher ein Backup exportieren.

## Standardbelegung der Referenzboards

| Funktion | ESP32 | C3 | C6 | S2 | S3 | ESP8266 |
|---|---:|---:|---:|---:|---:|---:|
| Motor-MOSFET | 25 | 10 | 10 | 14 | 4 | D5 / 14 |
| Jam, HIGH = Fehler | 26 | 4 | 11 | 15 | 5 | D6 / 12 |
| Lichtschranke | 27 | 5 | 18 | 16 | 6 | D7 / 13 |
| Hall-Sensor | 33 | 6 | 19 | 17 | 7 | D8 / 15 |
| Manueller Taster | 32 | 7 | 20 | 21 | 15 | D3 / 0 |
| DS3231 SDA | 21 | 0 | 6 | 8 | 8 | D2 / 4 |
| DS3231 SCL | 22 | 1 | 7 | 9 | 9 | D1 / 5 |
| Versorgungserkennung/ADC | 34 | 3 | 2 | 1 | 1 | A0 |
| einfache Status-LED | 2 | – | – | – | – | eingebaut |

Die adressierbaren RGB-LEDs vieler C-/S-DevKits sind keine einfachen digitalen
LEDs. Sie bleiben deshalb standardmäßig deaktiviert; die Statusanzeige der
Weboberfläche funktioniert unabhängig davon. Eine externe einfache LED kann mit
`FEEDMATE_PIN_STATUS_LED` aktiviert werden.

## Andere C-/S-Boards verwenden

Boardnamen wie SuperMini, XIAO, LOLIN, Feather oder QT Py bezeichnen andere
Platinenlayouts. Die Chipfamilie ist kompatibel, aber die GPIO-Zuordnung muss zum
Schaltplan der konkreten Platine passen. Dazu ein eigenes Ziel ergänzen:

```ini
[env:mein_c3_board]
extends = env:esp32c3
board = seeed_xiao_esp32c3
build_flags =
  -DFEEDMATE_CUSTOM_PIN_PROFILE=1
  -DFEEDMATE_PIN_MOTOR_GATE=10
  -DFEEDMATE_PIN_JAM_SENSOR=4
  -DFEEDMATE_PIN_LIGHT_BARRIER=5
  -DFEEDMATE_PIN_HALL_SENSOR=6
  -DFEEDMATE_PIN_MANUAL_BUTTON=7
  -DFEEDMATE_PIN_RTC_SDA=0
  -DFEEDMATE_PIN_RTC_SCL=1
  -DFEEDMATE_PIN_POWER_SENSE=3
  -DFEEDMATE_PIN_STATUS_LED=-1
```

Die Beispielwerte müssen anhand des konkreten Boards geprüft und bei Bedarf
geändert werden. Motor und Jam dürfen nie denselben Pin verwenden; SDA und SCL
müssen verschieden sein. Keine Flash-/PSRAM-, native USB- oder Boot-Strapping-
Pins verwenden. Das Motor-Gate benötigt weiterhin einen externen Pulldown, damit
der Motor während Reset und Boot sicher ausgeschaltet bleibt.

## Grenzen

„Alle Boards“ kann nur die von der festgeschriebenen Arduino-/PlatformIO-Version
erkannten Chipfamilien und Boarddefinitionen bedeuten. v1.24 enthält direkt
gebaute Ziele für ESP32, C3, C6, S2 und S3. Neuere Chipgenerationen ohne
Toolchain-Unterstützung sowie Platinen mit zu wenigen frei zugänglichen GPIOs
können nicht durch eine Board-ID allein sicher unterstützt werden.

Alle Plattformen arbeiten mit 3,3-V-Logik. ADC-Spannungsbereich und
Spannungsteiler müssen zum jeweiligen Board passen. Die Schwellen 450/550 sind
10-Bit-Rohwerte und keine Volt- oder Prozentangaben.
