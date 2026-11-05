# FeedMate – ESP32-/ESP8266-Firmware

## v1.14: Kompakte Geräteverwaltung

Die großen roten „Entfernen“-Schaltflächen wurden in der Automatenansicht durch kompakte Mülleimer-Symbole ersetzt. Die Symbole bleiben auf Smartphones rechts oben in der jeweiligen Gerätekarte und besitzen weiterhin eine verständliche Beschriftung für Screenreader.

## v1.13: Einheitliche Kacheltitel und ausblendbare Hinweise

Der leere Anmeldecontainer nach dem Login ist entfernt. Alle Hauptansichten verwenden Titelzeilen mit Info-Icon und – sofern Daten geladen werden – einem Aktualisieren-Icon rechts. Erklärungstexte sind standardmäßig verborgen und werden nur über das Info-Icon ein- oder ausgeblendet. Erfolg- und Fehlermeldungen verschwinden nach 4,5 Sekunden automatisch. Das ZIP enthält erneut das vollständige Projekt und die [Installationsanleitung v1.13](doc/INSTALLATION-v1.13.md).

## v1.12: Simulierte Automaten ausschließlich in der Demo

Die eigenständige Demo kann nicht existente Automaten hinzufügen und simuliert Status, Gruppenportionen und Zeitplanübertragung vollständig lokal. Die produktive `data/index.html` behält dagegen die echte Netzwerkprüfung. Das v1.12-ZIP enthält wieder das vollständige Projekt und eine ausführliche [Installationsanleitung](doc/INSTALLATION-v1.12.md).

## v1.11: Zeitstatus und sauberer mobiler Editor

Aktive Fütterungszeiten werden grün, inaktive rot hervorgehoben. Löschen erfolgt über ein Mülleimersymbol direkt auf Höhe der Aktiv-Checkbox. Alle Zeitplanfelder und die Automatenauswahl sind auf die verfügbare Kartenbreite begrenzt, damit rechts nichts abgeschnitten wird.

## v1.10: Zeitplan nach mobiler Vorlage

Die Zeitplanüberschrift trägt nun direkt einen Hilfe- und einen Aktualisieren-Iconbutton. Der bisher statische Erklärungstext erscheint ausschließlich im Hilfe-Popup. Eingeklappte Fütterungen zeigen zweizeilig nur Uhrzeit/Portion und die gewählten Automaten. Der aufgeklappte Editor ist auf Smartphones vollständig einspaltig. Der leere Anmeldebereich wird nach erfolgreicher Anmeldung ausgeblendet.

## v1.9: Header und Hilfe für die mobile Bedienung

Der Header bleibt wieder im normalen Dokumentfluss. Nach der Anmeldung zeigt er vollständig Statuslampe und Statuswort und enthält rechts daneben die Abmeldung. Hinweise werden über eine Hilfe-Schaltfläche geöffnet; die Demo ersetzt diesen Bereich durch ihren Demo-Hinweis. „Neu laden“ ist ein kompakter Sekundärbutton.

## v1.8: Smartphone-Navigation und klare Kurzansicht

Die Verwaltung öffnet auf kleinen Displays über ein Burger-Menü. Der Header bleibt beim Scrollen sichtbar und zeigt die Statuslampe. Eingeklappte Zeitkarten enthalten ausschließlich Uhrzeit, Automaten und Portion; Futter und weitere Einstellungen stehen erst im aufgeklappten Editor. Der Verlauf ist als einfache Ereignisliste dargestellt.

## v1.7: Übersichtliche Mehrfachauswahl

Jede Fütterungszeit kann jetzt mehreren Automaten zugewiesen werden. Die Automaten erscheinen als kompakte Checkboxen statt als einzelnes Auswahlfeld. Zeitkarten sind zunächst eingeklappt und zeigen nur Zeit, Automaten, Portion und Futtersorte; beim Aufklappen lassen sich Aktivierung, Sekunden, Automaten und Futter ändern. Die obere Navigation umbreitet sich auf Smartphones, statt eine horizontale Scrollleiste zu erzwingen.

## v1.6: Sekundengenauer Gruppenzeitplan

Der zentrale Smartphone-Zeitplan unterstützt `HH:MM:SS`, frei ergänzbare Zeilen und eine Automatenauswahl je Fütterung. Der gemeinsame Plan kann bis zu 96 Zeilen enthalten; beim Speichern werden höchstens 32 Einträge je Automat verteilt. So lassen sich mehrere Automaten gezielt um einige Sekunden versetzen. Die Zuordnung des gemeinsamen Plans liegt im Browser des Smartphones, während jeder Automat seine eigenen zugewiesenen Zeiten dauerhaft speichert.

Der Verlauf zeigt Ereignisse nun als chronologische, farblich gekennzeichnete Karten mit verständlichem Titel, Datum und Detailtext statt als JSON-Rohdaten. Abstände, Eingabefelder und Schaltflächen wurden erneut für schmale Smartphone-Displays angepasst.

## v1.5: Reparierte Demo-Navigation

Ein veralteter JavaScript-Handler für den in v1.4 entfernten Dauer-Button brach die Initialisierung nach Anmeldung und Übersicht ab. Der Handler ist entfernt; die Ansichtsumschaltung verwendet nun eine eindeutige aktive Klasse. Die Demo vermeidet außerdem `structuredClone`, damit sie auch in älteren Smartphone-Browsern funktioniert. Alle sieben Bereiche wurden nach Anmeldung automatisiert auf tatsächliche Sichtbarkeit geprüft.

## v1.4: Smartphone-Zentrale und klare Kalibrierung

Die Oberfläche trennt jetzt eindeutig **Zeitplan**, **Futter & Kalibrierung**, **Automaten**, **Backup** und **Verlauf**. Eine Statuslampe im Kopf zeigt sofort „alles okay“, Warnungen oder Gerätefehler. Die Futterkalibrierung führt in drei Schritten durch Probelauf, externes Wiegen und Speichern. Im Zeitplan werden nur Uhrzeit, Futtersorte und Grammportion gewählt; Laufzeit beziehungsweise Hall-Umdrehungen werden aus dem Futterprofil berechnet.

Mehrere Automaten lassen sich nach einmaliger Einrichtung eines gemeinsamen, mindestens 16 Zeichen langen Gruppenschlüssels von einem Smartphone aus auswählen, gemeinsam füttern und mit demselben Zeitplan versorgen. Die Geräte dürfen für gleichnamiges Futter unterschiedliche Kalibrierwerte besitzen. Adressen und Gruppenschlüssel liegen lokal im Browser des Smartphones; dieses Gerät daher schützen. Die Kommunikation läuft direkt im lokalen Netz und unverschlüsselt. Ein Automat muss erreichbar sein, und die Zeitplanübertragung wird bewusst per Knopfdruck ausgelöst – es gibt keinen Hintergrunddienst.

## v1.3: Interaktive Demo

[`demo.html`](demo.html) kann direkt auf einem Smartphone oder Computer geöffnet werden. Das Demo-Kennwort lautet **admin**. Die Datei simuliert Geräteantworten für Status, Probelauf, Portionen, Futtersorten, Zeitplan, Einstellungen, Verlauf und Backup. Änderungen werden ausschließlich im lokalen Browser-Speicher abgelegt und steuern keine Hardware. Der simulierte Wiederherstellungscode lautet `demo`; ein Passwortwechsel wird angezeigt, das Demo-Kennwort bleibt anschließend `admin`.

Zu jeder künftigen Version wird eine entsprechend benannte, eigenständige Demo-HTML zusammen mit dem Änderungs-ZIP ausgeliefert.

## v1.2: Portionen, Futtersorten und mehrere Geräte

Die Smartphone-Oberfläche zeigt Verwaltungsfunktionen erst nach Anmeldung. Eine Futtersorte wird durch einen Probelauf kalibriert: Laufzeit und optional gezählte Hall-Umdrehungen ablesen, die ausgegebene Futtermenge extern wiegen und als Profil speichern. Bis zu acht Profile können wiederverwendet werden. Danach kann eine gewünschte Grammmenge manuell oder im Zeitplan ausgewählt werden. **Gramm sind nur ein aus der Probe berechneter Richtwert; eine Waage ist nicht eingebaut.** Es gelten weiterhin maximal 30 Sekunden und die konfigurierte Drehgrenze. Bei einer nicht erreichbaren Portion wird kein Motorlauf gestartet. Zeitplanslots mit einer gelöschten oder ungeeigneten Kalibrierung werden als fehlgeschlagen protokolliert und am selben Tag nicht erneut versucht.

Ein authentifizierter JSON-Export enthält WLAN-/MQTT-Zugangsdaten, Zeitplan und Futterprofile. Diese Datei vertraulich behandeln. Ein Import ist in v1.2 noch nicht vorhanden; der Export dient als Sicherung und zur manuellen Wiederherstellung. Die Passwort-Hash-Salzung ist gerätespezifisch: Bei einem Gerätewechsel ein neues Admin-Passwort setzen.

Weitere FeedMate-Adressen können in der Smartphone-Oberfläche gespeichert werden. Die Adressen liegen **nur im lokalen Browser-Speicher** dieses Smartphones. Jeder Automat öffnet innerhalb der Übersicht seine eigene geschützte Oberfläche und verlangt eine eigene Anmeldung. Das ist eine Geräteauswahl; Fütterungen, Zeitpläne und Zugangsdaten werden nicht automatisch zwischen Automaten synchronisiert. Fremde Seiten können durch Browser- oder Netzwerkregeln das Einbetten verhindern; dann die Geräteadresse direkt öffnen.

FeedMate steuert einen Futterautomaten auf einem **ESP32 DevKit / ESP32-WROOM** (Standard) oder **WeMos D1 mini / ESP8266** mit DS3231, Motor-MOSFET und optionalen Sensoren. Diese Firmware verwendet Arduino-C++ und PlatformIO, kein ESPHome.

## Einstieg

1. [Installation und Migration](doc/HowToFlash.md) lesen; bestehende Flash-Daten sichern.
2. [Board und Pinbelegung](doc/BOARDS.md) auswählen. Mit PlatformIO `pio run -e esp32dev` und `pio run -e esp32dev -t buildfs` ausführen; für ESP8266 jeweils `-e d1_mini`.
3. Firmware und bei der Ersteinrichtung das LittleFS-Image per USB übertragen.
4. Seriellen Monitor mit 115200 Baud öffnen. Ein neues Admin-Passwort erscheint **einmalig**; bei jedem Start erscheint ein neuer Wiederherstellungscode. Das zufällige Setup-WLAN-Passwort erscheint bei jedem Start des Fallback-AP.
5. Im Setup-WLAN die Adresse http://192.168.4.1 öffnen, anmelden, WLAN und Zeitzone konfigurieren und neu starten.
6. Erst nach Prüfung der Motor- und Sensorfunktion Zeitpläne aktivieren. Neue und alte unmarkierte Zeitpläne sind zunächst deaktiviert.

## Verhalten

- Acht feste Zeitplan-Slots, lokale Uhrzeit nach POSIX-Zeitzonenregel. Vorgabe UTC; Deutschland: `CET-1CEST,M3.5.0,M10.5.0/3`.
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
| code/old/ | Unveränderte historische Dateien; vom Build ausgeschlossen |
| data/index.html | Aktuelle, zur API passende lokale Bedienoberfläche |
| tests/ | Tests der tatsächlichen Motor- und Zeitplanimplementierung mit simulierten Ein-/Ausgängen |
| doc/CHANGES.md | Änderungen, API- und MQTT-Vertrag, bekannte Grenzen |
| doc/TEST_REPORT.md | Ausgeführte Prüfungen und verbleibende Hardwaretests |
| doc/legacy/ | Ursprüngliche README und Flash-Anleitung, nicht die aktuelle Anleitung |

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
