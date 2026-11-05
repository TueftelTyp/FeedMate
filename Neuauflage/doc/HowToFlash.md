# Installation und Migration

## Build

PlatformIO Core 6.2.0 verwenden. `platformio.ini` pinnt Espressif32 6.10.0 / Arduino Core 2.0.17 (ESP32) und Espressif8266 4.2.1 / Arduino Core 3.1.2 (ESP8266), ArduinoJson 6.21.5, PubSubClient 2.8.0, RTClib 2.1.4 und Adafruit BusIO 1.16.1. Standardboard: esp32dev (ESP32-WROOM), zusätzlich d1_mini (ESP8266), jeweils 4 MB Flash und LittleFS. Die getrennte Verdrahtung steht in [BOARDS.md](BOARDS.md).

```sh
pio run -e esp32dev
pio run -e esp32dev -t buildfs
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
pio device monitor -b 115200
```

Für ESP8266 in allen Befehlen `esp32dev` durch `d1_mini` ersetzen.

Unter Windows kurze Projekt- und PlatformIO-Core-Pfade verwenden. Sehr lange Include-Pfade können beim älteren ESP32-GCC trotz vorhandener Compilerdateien zu `CreateProcess: No such file or directory` führen. Ein kürzerer `PLATFORMIO_CORE_DIR` behebt dieses Umgebungslimit; es ist kein Fehler des Firmwarecodes.

Bei Bedarf jeweils `--upload-port COMx` ergänzen. Zum ersten Start den seriellen Monitor bereits bereithalten und Reset betätigen. Nach einer erfolgreichen Erstinitialisierung wird das generierte Admin-Passwort nicht erneut ausgegeben. Es ist weder das WLAN-Passwort noch ein festes Standardpasswort.

## Achtung bei bestehenden Geräten

**uploadfs ersetzt das gesamte Dateisystem:** Konfiguration, Passworthash, Zeitplanreservierungen und Logs gehen ohne Sicherung verloren. Vorher Motorversorgung trennen und vorhandene Daten sichern. Bei normalem Firmwareupdate nur `upload` ausführen; das erhält LittleFS. Bei notwendigem UI-Update das Dateisystem einschließlich bestehender JSON-Dateien kontrolliert sichern und neu aufbauen. Sicherungen mit WLAN-/MQTT-Zugangsdaten nicht veröffentlichen.

Die Firmware formatiert niemals automatisch. Ein nicht mountbares Dateisystem sperrt den Motor. Ein beschädigter Zeitplan sperrt den Motor ebenfalls. Zur Wiederherstellung Daten über USB sichern, Fehler prüfen und gegebenenfalls ein frisches Dateisystem laden; danach sind alle Slots deaktiviert. Nicht während einer geplanten Fütterungsminute einrichten oder Reservierungen zurücksetzen.

Legacy-Zeitpläne ohne `lastRunDate` werden deaktiviert geladen. Jeden Slot prüfen und bewusst aktivieren. Alte Platzhalter-Passworthashes werden durch ein neues zufälliges Admin-Passwort ersetzt. Ein bereits vorhandener 64-stelliger Hash wird beibehalten; fremde Hash-Verfahren sind nicht automatisch kompatibel.

## Konfiguration über die Weboberfläche

Das Gerät startet bei fehlendem WLAN bzw. nach 30 Sekunden erfolgloser Verbindung einen Setup-AP mit gerätebezogenem Namen und zufälligem Passwort (USB-Ausgabe). Der AP bleibt bis zum nächsten Neustart aktiv. Im Setup-AP: http://192.168.4.1, sonst die Router-IP des Gerätes.

Anmelden, Einstellungen laden und JSON bearbeiten. Secrets sind in GET-Antworten nicht enthalten und können explizit ergänzt werden. Beispiel zusätzlicher Felder:

```json
{"wifiSsid":"MeinWLAN","wifiPassword":"hier-eintragen","timezone":"CET-1CEST,M3.5.0,M10.5.0/3"}
```

Nach Speichern das Gerät neu starten, um Netzwerk und Zeitzone konsistent anzuwenden. Ohne funktionsfähige RTC zunächst auf Netzwerkzeitsynchronisation warten und `clockValid` im Status prüfen. Fehlende RTC wird im Status angezeigt; eine bereits synchronisierte Systemuhr läuft bis zum nächsten Neustart weiter. Die RTC muss eine unabhängig erhaltene Versorgung besitzen, wenn sie einen vollständigen Stromausfall überbrücken soll. Hinweise der ursprünglichen Hardwareanleitung zu pauschalem Entfernen der RTC-Batterie sind keine Garantie für Offline-Zeit.

Passwortänderung: `adminPassword` als neues Klartextpasswort mit 16 bis 128 Zeichen mitsenden. Danach erneut anmelden. Bei Verlust des Passworts gibt es absichtlich keinen unauthentifizierten Netzwerk-Reset; eine kontrollierte Neueinrichtung über USB ist erforderlich.

## Erstprüfung ohne Futter

1. GPIO- und Sensorpegel einschließlich Boot-Strapping prüfen; Motor beim Einschalten sicher aus.
2. Kurzen manuellen Lauf prüfen, dann Jam-Signal auslösen und Abschaltung messen.
3. Hall-Sensor nur aktivieren, wenn er korrekt verdrahtet ist. 1500 ms ohne Impuls führen zum Fehler; Impulsabstände unter 2 ms werden ausgefiltert. Diese Werte müssen zur Mechanik passen.
4. RTC, Netzwerkzeit und lokale Zeitzone prüfen.
5. Einen Testslot anlegen und Neustartschutz ausprobieren; danach Produktivslots aktivieren.

Web- und MQTT-Steuerung ändern sich gegenüber dem alten Entwurf; Details stehen in CHANGES.md. Das historische `code/old` nicht flashen.
