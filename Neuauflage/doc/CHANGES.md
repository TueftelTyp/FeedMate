# Änderungen vom 05.11.2026

## Kompilierbarkeit und Struktur

- PlatformIO-Projekt mit festgelegten Abhängigkeiten, LittleFS und ESP32-Standardziel esp32dev sowie d1_mini-Ziel hinzugefügt.
- Gemeinsame Funktionsdeklarationen korrigiert; veraltetes code/old vom Build ausgeschlossen.
- Fehlerhafte MQTT-Topic-Verkettung und nicht terminierte Payload-Ausgabe entfernt.
- Aktuelle lokale Weboberfläche unter data/ bereitgestellt; ursprüngliche Dateien bleiben im Projektarchiv erhalten.
- GitHub-Actions-Build und portable C++17-Tests hinzugefügt.

## ESP32 und ESP8266

ESP32 ist das Standardziel. Eine gemeinsame Plattformabstraktion unterstützt native Webserver, SHA-256, Zufallsquellen, LittleFS-Mount und Zeitsynchronisation auf beiden Chips. Der ESP32 nutzt 240 MHz, ADC1 und eine eigene Pinbelegung ohne Boot-Strapping-Konflikte an Motor, Hall und Taster. Diagnose zeigt Plattform, Geräte-ID und freien Heap. Details und Grenzen: [BOARDS.md](BOARDS.md).

## Authentifizierung

- Beliebige Bearer-Tokens, vorhersagbare Session-Strings, leeres Standardpasswort und Platzhalterhash entfernt.
- Erstpasswort aus ESP-Zufallsquelle, gerätespezifisch gesalzener SHA-256, Token mit 256 Bit Zufall, konstanter Vergleich bei gleicher Länge, Ablauf nach 30 Minuten.
- Jede API außer Login fordert Authentifizierung. Neue Anmeldung ersetzt die vorherige Sitzung; Logout, Passwortwechsel und Reboot machen sie ungültig.
- Login höchstens einmal je zwei Sekunden. Falsche Typen, zu große JSON-Daten, falsche Slotanzahl und ungültige Zahlen werden zurückgewiesen.
- API gibt keine gespeicherten Zugangsdaten aus; kein pauschales CORS, Tokens nicht in URLs. Unsicheres Standard-OTA entfernt.
- Grenzen: kein HTTPS, kein MQTT-TLS, kein langsamer Passwort-KDF. Physischer Flash-Zugriff kann WLAN-/MQTT-Secrets lesen. Dies ist keine umfassende Sicherheitszertifizierung.

## Fütterungen, Speicherung und Motor

- Persistentes lastRunDate (YYYYMMDD) je Slot. Reservierung mit temporärer Datei, Länge, Leseprüfung und LittleFS-Rename vor Motorstart. Kein automatisches Formatieren.
- Reservierungsfehler sperrt alle Fütterungen bis zur Wiederherstellung. API-Änderungen werden bei fehlgeschlagenem Speichern im RAM zurückgenommen.
- Reservierung bleibt bei Fehler, Reboot und Slotänderung erhalten. Frühere Kalenderdaten werden nicht erneut ausgeführt; dadurch wird auch die doppelte Stunde bei Winterzeit pro Slot nur einmal bedient.
- Neue und Legacy-Slots sind zunächst deaktiviert. Keine Nachholfütterung. Eine Fütterung kann bei Stromausfall ausfallen; genau-einmal-Ausgabe ist ohne unabhängige Dosiermessung nicht garantiert.
- Ein zentraler Motorcontroller prüft Speicherstatus, aktiven Lauf, Jam-Latch, direkten Jam-Pegel, Dauer 1..30000 ms und Hall-Konfiguration.
- Web/MQTT nehmen zunächst eine Anfrage an; Motorstart folgt erst nach Rückkehr der Netzwerkverarbeitung. Während des Motorlaufs keine synchronen Netzwerk-/Flashzugriffe.
- Jam-Debounce 50..1000 ms, Laufzeitüberwachung mit millis-Überlaufverträglichkeit, getrennte Erfolg-/Abbruchereignisse. Abbruch meldet niemals gleichzeitig Erfolg.
- Taster entprellt; Fehlerquittierung startet keine Portion. Ein erneuter Tastendruck ist nötig.

## Uhr und Sensoren

- RTC-Presence, Power-Loss und Plausibilität geprüft. Kein Compile-Time-Ersatzdatum.
- Systemzeit wird aus vertrauenswürdigem RTC-Zustand bzw. SNTP gesetzt, RTC wird bei Abweichung korrigiert. UTC-Speicherung und POSIX-Zeitzone getrennt.
- RTC-Ausfall nach Boot wird durch I2C-Prüfung erkannt; synchronisierte Systemuhr bleibt bis Reboot verwendbar. NTP besitzt in diesem Stand keine kryptografische Authentifizierung.
- Hall-Interrupt mit 2-ms-Filter statt verlustanfälliger Loop-Flankenzählung. Fehlende Impulse nach 1500 ms sowie Überschreitung der Grenze verriegeln den Motor. Erreichen der Sollumdrehungen beendet den Lauf; Zeitablauf vor Soll ist Fehler.
- Lichtschranke liefert nach 500 ms stabilem Pegel Futter vorhanden/fehlt. Kein erfundener Prozentwert mehr in API oder MQTT; erneute Leer-Meldung nach Wiederbefüllung möglich.
- Stromversorgung mit ADC-Hysterese; keine erfundene Batteriespannung/-prozentangabe. Automatischer Deep Sleep entfernt.

## Logs

- Chronologische Ringpufferindizes bei teilweiser und voller Belegung korrigiert; Neustart setzt den Schreibindex hinter das letzte Element.
- UTC-Epoch und Boot-Uptime getrennt. Legacy-millis-Werte als Uptime importiert, deren UTC bleibt unbekannt.
- Ereignisse werden im Stillstand sofort atomar gespeichert; Ereignisse während Motorlauf bleiben bis zum Stopp im RAM. Fehlgeschlagene Speicherung bleibt als logsPending sichtbar und wird alle fünf Sekunden erneut versucht.
- Start wird vor Einschalten protokolliert. Scheitert dieser Commit, läuft der Motor nicht. Ein gespeicherter Start bedeutet eine beabsichtigte Aktivierung, keinen Nachweis tatsächlicher Bewegung.
- Die letzten 100 Ereignisse werden aufbewahrt; keine Umsetzung der alten tagebasierten logRetention-/Filteroptionen. Vollständige Log-Snapshots erzeugen Flash-Schreiblast; Dauereinsatz und freien Heap auf Hardware prüfen.
- Bei abruptem Stromverlust kann das letzte Ergebnis fehlen. Die separate Slotreservierung verhindert trotzdem einen erneuten Zeitplanversuch, sofern das Dateisystem erhalten bleibt. Physische Flashfehler oder manuelles Löschen/Zurückspielen alter Reservierungen liegen außerhalb dieser Garantie.

## API-Vertrag

JSON-Objekte, Authorization: Bearer TOKEN. Login liefert token und expiresIn. Keine Tokens in GET-Parametern.

| Methode/Pfad | Bedeutung |
|---|---|
| POST /api/login | {"password":"..."}; 200 oder 401, bei schnellen Wiederholungen 429 |
| POST /api/logout | Sitzung löschen |
| GET /api/status | Uhr-/RTC-/Speicherstatus, Fehler, Sensoren und Boot-Nonce |
| POST /api/feed | {"duration":3000}; 202 queued oder 409; tatsächliches Ergebnis im Log |
| GET/POST /api/schedule | slots mit exakt acht Objekten; active, hour, minute, duration; lastRunDate nur lesbar |
| GET/POST /api/config | Teilupdates; Secrets nur schreibbar; Netzwerk-/Zeitzonenänderungen durch Reboot anwenden |
| GET /api/logs | logs mit ts, uptime, type, msg in chronologischer Reihenfolge |
| POST /api/reset-fault | Nur bei inaktivem Jam-Eingang und stillstehendem Motor |

Während der Motor läuft, werden Webanfragen erst nach Abschaltung verarbeitet. Die Motorstatusanzeige ist deshalb kein Live-Telemetriekanal. Das dient der zeitnahen Sensor- und Laufzeitprüfung und ist eine bewusste Einschränkung des synchronen Webservers.

## MQTT-Vertrag – bewusst inkompatibel mit alten Steuer-Topics

Basis: `<mqttPrefix>/<Geräte-ID in Hex>`, z.B. `feedmate/abc123`.

- Verbindung mit Last Will auf status/online, 0 offline und 1 online, retained. Wiederverbindung mit 5..60 Sekunden Backoff; Socket-Wartezeit eine Sekunde; Paketpuffer 1024 Byte. DNS/TCP kann trotzdem im Stillstand blockieren und geplante Minuten verzögern.
- Geräte-ID in Discovery und Topics eindeutig. Home Assistant erhält einen binären Futter-Sensor; kein unsicherer Feed-Switch mit wiederholbarer fester Payload.
- Status enthält status/boot als zufällige Boot-Nonce. Steuerung nur auf control/feed mit JSON:

```json
{"boot":"Nonce aus status/boot","seq":1,"duration":3000}
```

- Pro Boot muss seq strikt steigen (uint32). Auch abgewiesene Anfragen verbrauchen die Sequenz. Mehrere Sender müssen eine gemeinsame Sequenz koordinieren.
- Gleiche Nachricht nach Wiederverbindung wird nicht erneut ausgeführt; Nachrichten früherer Boots passen nicht zur Nonce. Der Broker kann Retain-Flags in dieser Bibliothek nicht an den Callback melden. Deshalb niemals retained Steuerbefehle senden; eine zuvor noch nicht konsumierte Nachricht mit aktueller Nonce könnte sonst beim Wiederverbinden erstmals angenommen werden.
- Alte control/feed_now, feed_duration und reset_sensors werden nicht abonniert. Bestehende Automationen und alte Discovery-Einträge am Broker manuell aktualisieren/entfernen.
- MQTT selbst erfordert vertrauenswürdigen Broker und Topic-ACLs; die Nonce ist Replay-Schutz und kein Passwort. Motorstatus wird nur im Stillstand gesendet; direkte Laufstatus-/Ergebnisbestätigung über MQTT ist noch nicht implementiert, Ergebnisse sind über die Logs abrufbar.

## Technische Referenzen

Die verwendeten Bibliotheks-APIs wurden anhand der installierten, gepinnten Quellen sowie der offiziellen Dokumentation geprüft:

- [PubSubClient API](https://pubsubclient.knolleary.net/api): Puffer, Timeout und Publish-Rückgabewerte.
- [ESP8266 Arduino Core](https://arduino-esp8266.readthedocs.io/en/3.1.2/): Plattform und Netzwerk-/Dateisystemintegration.

Kein Hardwaretest und keine Cloud-Veröffentlichung wurden durchgeführt.
