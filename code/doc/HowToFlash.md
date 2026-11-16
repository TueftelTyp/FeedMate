# Installation und Migration

## Build

PlatformIO Core 6.2.0 verwenden. `platformio.ini` pinnt Espressif32 8.0.0 / Arduino Core 3.3.12 und Espressif8266 4.2.1 / Arduino Core 3.1.2 sowie die benötigten Bibliotheken. Fertige Ziele sind `esp32dev`, `esp32c3`, `esp32c6`, `esp32s2`, `esp32s3` und `d1_mini`. Die getrennte Verdrahtung und die Anpassung weiterer Boardlayouts stehen in [BOARDS.md](BOARDS.md).

```sh
pio run -e esp32dev
pio run -e esp32dev -t buildfs
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
pio device monitor -b 115200
```

In allen Befehlen `esp32dev` durch das tatsächlich verwendete Ziel ersetzen. Ein ESP32-C3 darf beispielsweise ausschließlich mit `-e esp32c3` gebaut und hochgeladen werden.

Unter Windows kurze Projekt- und PlatformIO-Core-Pfade verwenden. Sehr lange Include-Pfade können beim älteren ESP32-GCC trotz vorhandener Compilerdateien zu `CreateProcess: No such file or directory` führen. Ein kürzerer `PLATFORMIO_CORE_DIR` behebt dieses Umgebungslimit; es ist kein Fehler des Firmwarecodes.

Bei Bedarf jeweils `--upload-port COMx` ergänzen. Zum ersten Start den seriellen Monitor bereits bereithalten und Reset betätigen. Das feste AP-Kennwort lautet `feedmate`, das feste Admin-Erstkennwort `FeedMateAdmin`. Das Admin-Kennwort nach der ersten Anmeldung ändern. Der serielle Monitor zeigt zusätzlich bei jedem Start einen neuen einmaligen Wiederherstellungscode.

## Achtung bei bestehenden Geräten

**uploadfs ersetzt das gesamte Dateisystem:** Konfiguration, Passworthash, Zeitplanreservierungen und Logs gehen ohne Sicherung verloren. Vorher Motorversorgung trennen und vorhandene Daten sichern. Bei normalem Firmwareupdate nur `upload` ausführen; das erhält LittleFS. Bei notwendigem UI-Update das Dateisystem einschließlich bestehender JSON-Dateien kontrolliert sichern und neu aufbauen. Sicherungen mit WLAN-/MQTT-Zugangsdaten nicht veröffentlichen.

Die Firmware formatiert niemals automatisch. Ein nicht mountbares Dateisystem sperrt den Motor. Ein beschädigter Zeitplan sperrt den Motor ebenfalls. Zur Wiederherstellung Daten über USB sichern, Fehler prüfen und gegebenenfalls ein frisches Dateisystem laden; danach sind alle Slots deaktiviert. Nicht während einer geplanten Fütterungsminute einrichten oder Reservierungen zurücksetzen.

Legacy-Zeitpläne ohne `lastRunDate` werden deaktiviert geladen. Jeden Slot prüfen und bewusst aktivieren. Bei fehlendem oder ungültigem Admin-Hash wird das feste Erstkennwort `FeedMateAdmin` eingerichtet. Ein bereits vorhandener 64-stelliger Hash wird beibehalten; fremde Hash-Verfahren sind nicht automatisch kompatibel.

## Konfiguration über die Weboberfläche

Das Gerät startet bei jedem Boot das dauerhaft verfügbare Verwaltungs-AP `FeedMate-<MAC-Adresse>` mit Kennwort `feedmate`. Es bleibt parallel zu einer erfolgreichen Heimnetzverbindung aktiv und wird automatisch überwacht. Adresse im AP: http://192.168.4.1; im Heimnetz ist zusätzlich die Router-IP nutzbar.

Anmelden und die beschrifteten Felder der Smartphone-Oberfläche verwenden. Kennwörter werden beim Laden nicht zurück an den Browser übertragen; leere Kennwortfelder lassen gespeicherte Werte unverändert. Die API akzeptiert beispielsweise:

```json
{"wifiSsid":"MeinWLAN","wifiPassword":"hier-eintragen","timezone":"Europe/Berlin"}
```

Nach Speichern das Gerät neu starten, um Netzwerk und Zeitzone konsistent anzuwenden. Ohne funktionsfähige RTC zunächst auf Netzwerkzeitsynchronisation warten und `clockValid` im Status prüfen. Fehlende RTC wird im Status angezeigt; eine bereits synchronisierte Systemuhr läuft bis zum nächsten Neustart weiter. Die RTC muss eine unabhängig erhaltene Versorgung besitzen, wenn sie einen vollständigen Stromausfall überbrücken soll. Hinweise der ursprünglichen Hardwareanleitung zu pauschalem Entfernen der RTC-Batterie sind keine Garantie für Offline-Zeit.

Passwortänderung: In den Einstellungen ein neues Admin-Passwort mit 16 bis 128 Zeichen eintragen. Danach erneut anmelden. Bei Verlust des Passworts Gerät über USB verbinden, seriellen Monitor (115200 Baud) öffnen und neu starten. Der Start zeigt einen neuen `RECOVERY CODE FOR THIS BOOT`. Auf dem Smartphone „Passwort vergessen?“ öffnen, Code und neues Passwort eingeben. Der Code gilt nur bis zum nächsten Neustart, höchstens fünf falsche Versuche sind möglich, und ein erfolgreicher Reset verbraucht ihn. Für einen erneuten Versuch nach Verbrauch des Codes physisch neu starten. Das bisherige Passwort wird nicht im Klartext wieder ausgegeben.

## Erstprüfung ohne Futter

1. GPIO- und Sensorpegel einschließlich Boot-Strapping prüfen; Motor beim Einschalten sicher aus.
2. Kurzen manuellen Lauf prüfen, dann Jam-Signal auslösen und Abschaltung messen.
3. Hall-Sensor nur aktivieren, wenn er korrekt verdrahtet ist. 1500 ms ohne Impuls führen zum Fehler; Impulsabstände unter 2 ms werden ausgefiltert. Diese Werte müssen zur Mechanik passen.
4. RTC, Netzwerkzeit und lokale Zeitzone prüfen.
5. Einen Testslot anlegen und Neustartschutz ausprobieren; danach Produktivslots aktivieren.

Web- und MQTT-Steuerung sind in CHANGES.md beschrieben.
