# Testbericht – 05.11.2026

## Umfang

Bearbeitet wurde die tatsächlich bereitgestellte FeedMate-main.zip, nicht nur der öffentliche GitHub-Stand. Ursprüngliche Dateien, Lizenz und historische Varianten bleiben enthalten. Aktive Firmware, Buildkonfiguration, Weboberfläche, Tests und Dokumentation wurden überarbeitet. Auf Wunsch ist ESP32 jetzt Standardziel; ESP8266 bleibt unterstützt.

Die Prüfungen liefen auf Windows ohne angeschlossene FeedMate-Hardware. Eine Buildfreigabe ist keine Hardwarefreigabe.

## Ausgeführte Prüfungen

| Prüfung | Ergebnis |
|---|---|
| ESP32-WROOM/DevKit, `pio run -e esp32dev` | Erfolgreich kompiliert und gelinkt, Firmware-Image erzeugt |
| ESP8266 D1 mini, `pio run -e d1_mini` | Erfolgreich kompiliert und gelinkt, Firmware-Image erzeugt |
| C++17-Tests der Produktionslogik | 54 Assertions bestanden |
| JavaScript der ausgelieferten Weboberfläche | Syntaxprüfung mit Node.js bestanden |

Dateisystem-Builds und endgültige Speicherwerte werden in `validation/SUMMARY.txt` ausgewiesen. Die Buildprotokolle liegen ebenfalls unter `validation/`.

Verwendet: PlatformIO Core 6.2.0, Espressif32 6.10.0 / Arduino 2.0.17 / Xtensa GCC 8.4.0, Espressif8266 4.2.1 / Arduino 3.1.2 / Xtensa GCC 10.3.0. ArduinoJson 6.21.5, PubSubClient 2.8.0, RTClib 2.1.4, Adafruit BusIO 1.16.1. Native Tests: Zig 0.13.0, C++17. Die Assertions waren aktiviert.

## Was die 54 Assertions abdecken

Die Tests binden `code/sensors.cpp` und `code/schedule.cpp` direkt ein. GPIO, Zeit und Commit-Ergebnis werden simuliert; die Zustandsmaschine wird nicht als zweite Testimplementierung nachgebaut. `code/logic.h` wird direkt verwendet.

- Dauer 0 und über 30 Sekunden abgewiesen; weiterer Start verändert keinen aktiven Lauf.
- Zeitgesteuerter Abschluss beendet den Motor und meldet Erfolg genau auf dem Erfolgsweg.
- Jam-Debounce, Start bei aktivem Jam-Signal, verriegelter Fehler und gesonderte Quittierung ohne Neustart.
- Fehlende Hall-Impulse, Zielumdrehungen, Überschreitung der Höchstzahl, Zeitablauf vor Ziel.
- Nicht verfügbarer Speicher und fehlgeschlagener Startlog-Commit verhindern Motorstart.
- Web/MQTT-Anforderung startet erst nach Bearbeitung der Warteschlange; doppelte Anforderung abgewiesen; Sensorfehler zwischen Anfrage und Start erneut geprüft.
- Ungültige Uhrzeit sperrt Zeitplan; fehlgeschlagene Reservierung sperrt Motor.
- Simulierter Reboot mit Neuladen der persistierten Reservierung in derselben Minute löst keine zweite Fütterung aus.
- Neuer Tag erlaubt den nächsten Versuch; Rücksprung auf vorherigen Tag nicht.
- Zwei gleichzeitige Slots überschreiben keine Laufzeit und werden nacheinander bedient.
- `millis()`-Überlauf beendet den Motor korrekt; Datum über Jahreswechsel und Ringpufferindizes geprüft.
- Lichtschranke leer/voll/erneut leer nach stabiler Pegelzeit.
- Authentifizierungshelfer lehnt leere Sitzung, beliebiges Token, fehlenden Header, Token mit Zusatz und abgelaufene Sitzung ab. Exaktes Token und Ablaufprüfung über Timerüberlauf funktionieren.

Die Tests prüfen **nicht** den tatsächlichen LittleFS-Flash-Commit, die BearSSL-/mbedTLS-Implementierung, das HTTP-Parsing des Servers, die elektrische ISR-Latenz oder Netzwerkpakete eines echten Brokers. Die Authentifizierungsprüfung betrifft den verwendeten Token-/Ablaufhelfer; keine Behauptung eines vollständigen Penetrationstests.

## Festgestellte und behobene Buildhindernisse

- Die vorherige Header-/Funktionsstruktur und MQTT-Topic-Verkettung waren nicht kompilierbar.
- ESP8266 und ESP32 besitzen unterschiedliche Webserver-Headerregistrierung, Dateisysteminitialisierung, Zufallszahl-, Hash- und Zeit-APIs; die Plattformschicht berücksichtigt diese Unterschiede.
- Die vorhandene lokale Visual-Studio-Installation hatte keine C++-Standardheader. Die nativen Tests wurden deshalb mit einem portablen Zig-Compiler gebaut und ausgeführt.
- Der erste ESP32-Build scheiterte am Windows-Kommandozeilenlimit durch sehr lange Toolpfade. Mit kurzem PlatformIO-Core-Pfad erfolgreich gebaut. Dies erforderte keine Firmware-Ausnahme.
- PubSubClient 2.8.0 erzeugt beim ESP8266-Build eine bekannte Signedness-Compilerwarnung in der Bibliothek; kein Buildfehler. Beim erstmaligen Zig-Runtime-Build entstanden Warnungen in libunwind, nicht im Test- oder Firmwarecode.

## Hardware- und Integrationstests vor Einsatz

1. **Boot/Reset:** Motor bleibt ohne Impuls aus; Brownout und Reset während aktivem Lauf messen. Externe Gate-Pulldown-/Freilaufbeschaltung prüfen.
2. **Stromausfall:** Wiederholt unmittelbar vor/nach Reservierung, während Motorlauf und während Log-Commit abschalten. Keine erneute Slot-Ausführung in derselben Minute; unvollständige Fütterung darf als ausgelassen gelten. Flash-Inhalt und Logs kontrollieren.
3. **Dateisystem:** Voll, beschädigt und nicht mountbar simulieren; keine automatische Formatierung, keine Fütterung bei Reservierungsfehler. Rename-/Stromausfallverhalten auf beiden Plattformen prüfen.
4. **RTC/SNTP:** RTC fehlt, Batterie/Versorgung verloren, I2C unterbrochen, NTP ausgefallen, große Uhrkorrektur, Mitternacht, Monatswechsel und Sommer-/Winterzeitwechsel testen. Keine Plan-Ausführung bei unbekannter Zeit.
5. **Motor/Sensoren:** Jam bei Start und im Lauf, abgeklemmter Jam-Eingang, Hall dauerhaft HIGH/LOW, Impulsprellen und reale Drehzahlgrenzen testen. Die 1500-ms-Lücke muss zur Mechanik passen. Reale Futtermenge separat kalibrieren.
6. **HTTP:** Ohne Auth und mit fremdem/abgelaufenem Token alle Endpunkte prüfen; Passwortwechsel, Loginrate, JSON-Typen, zu große Bodies, ungültige Slots und fehlgeschlagene Konfigurationsspeicherung prüfen. Browser und langsame Clients während Motorlauf testen.
7. **MQTT:** Broker aus/ein, DNS-Fehler, fehlende ACL, Publish-/Subscribe-Fehler, große Payload, gleiche Sequenz, alte Boot-Nonce und retained Nachricht testen. Mehrere Geräte dürfen keine Topics/Discovery-IDs teilen. Alte Automationen auf den neuen Vertrag migrieren.
8. **Langlauf:** Heap und Dateisystemfüllung bei vollen 100 Logeinträgen überwachen. WLAN-Ausfälle, überlappende Slots und viele Requests über mehrere Tage prüfen; Flash-Schreiblast bewerten.
9. **Boardwechsel:** Richtige Pinbelegung, ADC-Spannungsteiler, LED-Polarität und Boot-Strapping prüfen. ESP32-C3/S3 sind nicht durch den ESP32-WROOM-Build mitgetestet.

Kein Upload auf ein Gerät, kein echter Broker-Test, kein GitHub-Actions-Lauf und kein Hardware-Dauertest wurden ausgeführt. Das aktualisierte Projekt ist ein getesteter Build- und Logikstand mit dokumentierter verbleibender Hardwarevalidierung.
