# FeedMate

**Make feeding your pet smart.**  

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC--BY--NC--SA--4.0-c38041?style=flat&labelColor=555555)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32%20%7C%20ESP8266-orange?style=flat&labelColor=555555)](code/platformio.ini)
[![Demo](https://img.shields.io/badge/Interactive-Demo-42c44a?style=flat&labelColor=555555)](https://tuefteltyp.github.io/demo/feedmate.html)

[Deutsch](#deutsch) | [English](#english)

<a id="deutsch"></a>
## Deutsch

FeedMate ist eine Smartphone-optimierte Firmware zur Steuerung eines Futterautomaten. Die aktuelle Codebasis verwendet Arduino und PlatformIO und unterstützt ESP32-Familien sowie ESP8266.

### Inhaltsverzeichnis

- [Funktionen](#de-funktionen)
- [Schnellstart](#de-schnellstart)
- [Projektverzeichnis](#de-projektverzeichnis)
- [Boards und Pinbelegung](#de-boards)
- [Demo](#de-demo)
- [Aktuelle Version](#de-version)
- [Lizenz](#de-lizenz)

<a id="de-funktionen"></a>
### Funktionen

- Geschützte, mobil optimierte Verwaltungsoberfläche mit Statusanzeige
- Sekundengenaue Fütterungspläne und wiederverwendbare Futterprofile
- Kalibrierung über Probelauf, Laufzeit oder Hall-Sensor-Umdrehungen
- Verwaltung mehrerer Automaten und gemeinsamer Zeitpläne
- RTC-/Netzwerkzeitsynchronisierung, MQTT, Protokoll und Backup
- C3 SuperMini als PlatformIO-Standardziel; weitere ESP32- und ESP8266-Ziele verfügbar

<a id="de-schnellstart"></a>
### Schnellstart

1. Lies die [Installationsanleitung](code/doc/INSTALLATION-v1.35.md) und prüfe die [Board- und Pinbelegung](code/doc/BOARDS.md).
2. Öffne den Ordner `code` als PlatformIO-Projekt.
3. Für einen vollständigen Neu-Flash des ESP32-C3 SuperMini in PowerShell ausführen:

   ```powershell
   .\scripts\flash-c3-supermini.ps1 -Port COM6
   ```

   Das löscht auch LittleFS mit gespeicherten Einstellungen, Zeitplänen und Logs. Der COM-Port kann abweichen.

4. Verbinde dich mit `FeedMate-<MAC-Adresse>` (Kennwort `feedmate`) und öffne `http://192.168.4.1`. Das erste Admin-Kennwort lautet `FeedMateAdmin`.

<a id="de-projektverzeichnis"></a>
### Projektverzeichnis

| Pfad | Inhalt |
|---|---|
| [`code/`](code/) | Aktives PlatformIO-Projekt |
| [`code/code/`](code/code/) | Firmware-Quellcode: WLAN, API, Motor, Sensoren, Zeitplan und Speicher |
| [`code/data/`](code/data/) | Weboberfläche, die in LittleFS gespeichert wird |
| [`code/doc/`](code/doc/) | Installations-, Board-, Änderungs- und Prüfberichte |
| [`code/scripts/`](code/scripts/) | Hilfsskripte, einschließlich Komplett-Flash für den C3 SuperMini |
| [`code/tests/`](code/tests/) | Controller-Regressionstests |
| [`code/platformio.ini`](code/platformio.ini) | PlatformIO-Umgebungen und Standardziel |
| [`code/demo.html`](code/demo.html) | Lokal ausführbare interaktive Demo |
| [`version/`](version/) | Versionierte Projekt-ZIP-Dateien |
| [`.old/`](.old/) | Archivierte Altdateien |

<a id="de-boards"></a>
### Boards und Pinbelegung

Verfügbare Ziele und Sicherheitshinweise stehen in [BOARDS.md](code/doc/BOARDS.md). Für andere Platinen derselben Chipfamilie kann eine angepasste Board-ID und Pinbelegung nötig sein.

<a id="de-demo"></a>
### Demo

Die [gehostete Demo](https://tuefteltyp.github.io/demo/feedmate.html) oder [`code/demo.html`](code/demo.html) lokal im Browser öffnen. Demo-Kennwort: `admin`. Die Demo steuert keine Hardware.

<a id="de-version"></a>
### Aktuelle Version

Aktueller Quellstand: **v1.35**. Das [vollständige ZIP](version/FeedMate_v1.35.zip) enthält das Projekt ohne alte Build-Artefakte.

<a id="de-lizenz"></a>
### Lizenz

FeedMate steht unter [CC BY-NC-SA 4.0](LICENSE).

<a id="english"></a>
## English

FeedMate is smartphone-friendly firmware for a pet food dispenser. The current codebase uses Arduino and PlatformIO and supports ESP32 families as well as ESP8266.

### Contents

- [Features](#en-features)
- [Quick start](#en-quick-start)
- [Project directory](#en-project-directory)
- [Boards and pinout](#en-boards)
- [Demo](#en-demo)
- [Current version](#en-version)
- [License](#en-license)

<a id="en-features"></a>
### Features

- Password-protected, mobile-friendly management interface with device status
- Second-accurate feeding schedules and reusable food profiles
- Calibration by timed test run or Hall-sensor rotations
- Multiple-device management and shared schedules
- RTC/network time synchronization, MQTT, event log, and settings backup
- ESP32-C3 SuperMini as the default PlatformIO target, with other ESP32 and ESP8266 targets available

<a id="en-quick-start"></a>
### Quick start

1. Read the [installation guide](code/doc/INSTALLATION-v1.35.md) and check the [board and pinout guide](code/doc/BOARDS.md).
2. Open the `code` directory as a PlatformIO project.
3. To completely reflash an ESP32-C3 SuperMini, run this in PowerShell:

   ```powershell
   .\scripts\flash-c3-supermini.ps1 -Port COM6
   ```

   This also erases LittleFS, including saved settings, schedules, and logs. Your serial port may differ.

4. Connect to `FeedMate-<MAC address>` (password `feedmate`) and open `http://192.168.4.1`. The initial admin password is `FeedMateAdmin`.

<a id="en-project-directory"></a>
### Project directory

| Path | Contents |
|---|---|
| [`code/`](code/) | Active PlatformIO project |
| [`code/code/`](code/code/) | Firmware source: Wi-Fi, API, motor, sensors, schedule, and storage |
| [`code/data/`](code/data/) | Web interface stored in LittleFS |
| [`code/doc/`](code/doc/) | Installation, board, change, and test documentation |
| [`code/scripts/`](code/scripts/) | Helper scripts, including the C3 SuperMini full-flash script |
| [`code/tests/`](code/tests/) | Controller regression tests |
| [`code/platformio.ini`](code/platformio.ini) | PlatformIO environments and default target |
| [`code/demo.html`](code/demo.html) | Interactive demo for a local browser |
| [`version/`](version/) | Versioned project ZIP files |
| [`.old/`](.old/) | Archived legacy files |

<a id="en-boards"></a>
### Boards and pinout

See [BOARDS.md](code/doc/BOARDS.md) for supported targets and wiring notes. Other boards using the same chip family may need a custom board ID and pin mapping.

<a id="en-demo"></a>
### Demo

Open the [hosted demo](https://tuefteltyp.github.io/demo/feedmate.html) or [`code/demo.html`](code/demo.html) in a browser. Demo password: `admin`. The demo does not control hardware.

<a id="en-version"></a>
### Current version

Current source: **v1.35**. The [complete ZIP](version/FeedMate_v1.35.zip) contains the project without old build artifacts.

<a id="en-license"></a>
### License

FeedMate is licensed under [CC BY-NC-SA 4.0](LICENSE).
