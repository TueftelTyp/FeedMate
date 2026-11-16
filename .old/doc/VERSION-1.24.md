# FeedMate v1.24

v1.24 korrigiert den auf ESP32 byteweise rückwärts aufgebauten Gerätenamen.
Das Verwaltungs-WLAN des geprüften ESP32-C3 heißt nun beispielsweise
`FeedMate-44bd8d5e19e8` statt `FeedMate-e8195e8dbd44`.

Der AP wird ausdrücklich auf Kanal 1, sichtbar und mit höchstens vier Clients
angelegt. Eine zweisekündliche Zustandsprüfung kontrolliert WLAN-Modus, IP und
SSID und stellt den AP bei einem Ausfall wieder her. Der AP-Status erscheint
zusätzlich in der Übersicht und in der geschützten Status-API. Für ESP32-C3
und C6 ist die native USB-Konsole aktiviert, damit Start, Wiederherstellung und
AP-Diagnose am verwendeten USB-Serial/JTAG-Anschluss sichtbar werden.

Vorhandene Admin- und Gruppenschlüssel bleiben gültig: Nur die öffentliche
Geräte-ID wurde korrigiert; der historische gerätespezifische Hash-Salt wird
absichtlich unverändert weiterverwendet.

Weitere Korrekturen betreffen validierte Zeitzonen, Sicherheitsheader der
Weboberfläche, den Backup-Formatbezeichner und eine warnungsfreie
Hall-ISR-Inkrementierung.
