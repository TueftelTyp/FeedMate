# Prüfbericht FeedMate v1.24

Stand: 12. November 2026

## Geprüfter Fehlerfall

Der angeschlossene ESP32-C3 auf COM6 wurde unabhängig vom Quellcode geprüft.
esptool erkannte einen ESP32-C3 QFN32 Revision 0.4 mit 4 MB Flash und der MAC
`44:bd:8d:5e:19:e8`. Das vollständige 4-MB-Flashabbild, App-Image,
Bootloader und Partitionstabelle waren lesbar und hatten gültige Prüfsummen.
Die geflashte App stimmte bytegenau mit dem v1.23-Build überein. Die
Coredump-Partition war leer und ein 20-sekündiger Bootmonitor zeigte keine
Reset-Schleife.

Die NVS-Daten bewiesen, dass v1.23 den AP initialisiert hatte. Dort war jedoch
`FeedMate-e8195e8dbd44` gespeichert. Ursache war die Ausgabe des 64-Bit-eFuse-
Wertes in Host-Byte-Reihenfolge. v1.24 liest die kanonische WLAN-MAC und erzeugt
für dieses Board `FeedMate-44bd8d5e19e8`.

## Reparaturen und Sicherheitsprüfung

- Korrekte, getestete MAC-Formatierung für die öffentliche Geräte-ID.
- Historischer Hash-Salt bleibt erhalten; bestehende Admin- und
  Gruppenschlüssel werden durch die Korrektur nicht ungültig.
- AP explizit sichtbar auf Kanal 1, Adresse `192.168.4.1`, maximal vier Clients.
- Zustandsprüfung alle zwei Sekunden anhand von Modus, IP und SSID; automatischer
  Neustart des AP bei Abweichung.
- Kontrollierte WLAN-Neuinitialisierung, damit ein geänderter Hostname vor dem
  Start der Station gesetzt und das Verwaltungs-AP anschließend sicher wieder
  aktiviert wird.
- AP-Zustand, SSID, IP, Clients sowie Start-/Fehlerzähler in der geschützten
  Status-API und Smartphone-Übersicht.
- Native USB-Konsole für ESP32-C3/C6 aktiviert.
- Zeitzonen auf die angebotenen IANA-Namen und bekannte Altdaten begrenzt;
  unbekannte Freitexte werden abgelehnt.
- Sicherheitsheader für HTML ergänzt; keine automatische Dateisystemformatierung,
  keine OTA-Schnittstelle mit Standardkennwort.
- Backup-Format auf `FeedMate-v1.24` aktualisiert.
- CORS von pauschalem `*` auf Same-Origin sowie Fleet-API-Aufrufe mit
  Gruppenschlüssel beschränkt; unbekannte Cross-Origin-Preflights abgewiesen.
- Scheduler führt beim Hochfahren keine bereits verstrichene Sekunde der
  aktuellen Minute nachträglich aus.
- Fremdbibliothekswarnung beim ESP8266-Build gezielt unterdrückt; der aktuelle
  vollständige Build läuft ohne Warnung durch.

## Automatisierte Prüfungen

| Prüfung | Ergebnis |
|---|---|
| 72 C++-Assertions: Motor, Jam, Hall, Sensor, Portion, Auth-Session, Timerüberlauf, Neustartreservierung, Speicherfehler, MAC- und HTTP-Headerformate | bestanden |
| JavaScript-Syntax von `data/index.html` | bestanden |
| JavaScript-Syntax von `demo.html` | bestanden |
| ESP32-C3, Arduino-ESP32 3.3.12: alle sechs Produktionsmodule mit echten Board-Headern | bestanden |
| ESP32-C6, Arduino-ESP32 3.3.12: alle sechs Produktionsmodule mit echten Board-Headern | bestanden |
| ESP8266 D1 mini: alle sechs Produktionsmodule | bestanden |
| Vollständiger ESP8266-Build einschließlich Link und BIN nach API-/CORS-Änderungen | bestanden, 48.048 B RAM / 407.207 B Flash |
| ESP32-C3 LittleFS-Image mit aktueller Oberfläche | bestanden |
| ESP8266 LittleFS-Image mit aktueller Oberfläche | bestanden |

Die ESP32-C3/C6-Quellprüfungen verwenden exakt die von PlatformIO 8.0.0
erzeugten Kommandos und Arduino-ESP32 3.3.12. Der vollständige Linklauf dieser
neuen GCC-14-Zielumgebungen kann innerhalb der abgeschotteten Prüfumgebung nicht
abgeschlossen werden, weil der Assembler beim Ermitteln seines eigenen Pfades mit
Windows-Fehler 5 abbricht. Das ist ein Werkzeug-/Umgebungsfehler, kein
Compilerbefund im FeedMate-Code. Der identische ESP32-C3-Werkzeugstand hat v1.23
außerhalb dieser Umgebung bereits vollständig gebaut, gelinkt und hochgeladen.

Beim abschließenden ESP8266-Clean-Build wurde zunächst eine nicht portable
Header-Signatur entdeckt (`int` statt `size_t`); der Aufruf ist nun auf die
gemeinsame `size_t`-Schnittstelle festgelegt und der vollständige Build danach
erfolgreich wiederholt worden.

## Verbleibende Hardwaregrenzen

- Ein realer AP-Funknachweis von v1.24 erfordert Flashen auf das Board und eine
  WLAN-Suche mit einem zweiten Gerät. Das wird in der Installationsanleitung als
  abschließender Abnahmetest beschrieben.
- Motorstrom, MOSFET-Pegel, Hall-Polarität, Lichtschranke und ADC-Teiler können
  ohne die konkrete Verdrahtung nicht elektrisch geprüft werden. Vor einem
  Motoranschluss muss die Pinbelegung aus `BOARDS.md` verifiziert werden.
- Das Verwaltungs-AP bleibt auf ausdrücklichen Wunsch dauerhaft aktiv. Das
  feste AP-Kennwort ist bequem, aber weniger sicher als ein individuelles
  Kennwort; das Admin-Kennwort muss nach der Ersteinrichtung geändert werden.
