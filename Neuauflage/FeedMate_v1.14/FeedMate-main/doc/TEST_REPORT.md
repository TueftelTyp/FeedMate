# Testbericht – 05.11.2026

## Ergänzung v1.14

- Produkt- und Demooberfläche zeigen in Geräte-/Automatenkarten ausschließlich kompakte Mülleimer-Symbole.
- Smartphone-Raster geprüft: Symbol rechts oben, Statuslampe separat, keine breite Entfernen-Zeile.
- Löschaktion und zugängliche Beschriftung im Browser geprüft.

## Ergänzung v1.13

Produkt- und Demo-JavaScript bestanden die Syntaxprüfung. Der Browser-Test bestätigte sieben Info-Buttons, zunächst verborgene und anschließend geöffnete Hinweise, fünf kompakte Aktualisieren-Icons, fehlende breite Laden-/Aktualisieren-Buttons, ausgeblendeten Anmeldecontainer und eine nach fünf Sekunden leere Meldungszeile. Alle Ansichten, Zeitplanfunktionen, Ereignisliste und simulierte Demo-Automaten wurden erneut geprüft.

## Ergänzung v1.12

Demo-JavaScript-Syntax bestanden. Im Browser wurde ein nicht existentes Gerät `Demo Küche` unter der reservierten Testadresse `http://192.0.2.42` hinzugefügt. Es wurde ohne Netzwerkanfrage als verbunden angezeigt und erschien danach als auswählbarer Automat im gemeinsamen Zeitplan. Die produktive Netzwerkfunktion wurde für v1.12 nicht verändert.

## Ergänzung v1.11

Syntax- und Browserprüfung bestanden. Bei 390 Pixel Smartphone-Breite wurde eine Zeitkarte geöffnet und deaktiviert. Geprüft wurden rote Inaktiv-Markierung, Mülleimersymbol, sofortige Zustandsänderung sowie die rechten Kanten von Uhrzeitfeld und Automatenbox. Beide lagen 13 Pixel innerhalb der Kartenkante und wurden nicht abgeschnitten.

## Ergänzung v1.10

JavaScript-Syntax von Produktoberfläche und Demo bestanden. Der Browser-Test bestätigt alle sieben Ansichten, mehr als acht Zeitkarten, Sekundenfeld, Automatenauswahl, vereinfachten Verlauf und Version v1.10. Zusätzlich geprüft: Der Hilfe-Iconbutton öffnet den vollständigen Zeitplanhinweis, Aktualisieren besitzt die kompakte Iconklasse und die eingeklappte Zusammenfassung enthält keine Futtersorte.

## Ergänzung v1.9

Die produktive Oberfläche und daraus erzeugte Demo bestanden die JavaScript-Syntaxprüfung. Der Browser-Test bestätigt Anmeldung, alle sieben Ansichten, Statuswort „Alles okay“, Versionsanzeige v1.9 und die neue Verlaufsdarstellung. Der Header enthält den Logout-Button, die Hilfe ist als eigenes Bedienelement vorhanden und der Zeitplan-Aktualisierer verwendet die kompakte Button-Klasse. Der Header ist nicht sticky.

## Ergänzung v1.8

Die Demo-JavaScript-Syntax wurde geprüft. Der Browser-Test bestätigt nach Anmeldung alle sieben Ansichten, Zeitkarten mit Sekunden und Checkbox-Automaten, den vereinfachten Verlauf und die Versionsanzeige v1.8. Die mobile CSS-Struktur verwendet keinen horizontalen Tab-Overflow mehr; der Header bleibt sticky. Die Menü-Umschaltung ist als eigener DOM-Handler vorhanden.

## Ergänzung v1.7

Produktoberfläche und daraus erzeugte Demo bestanden die JavaScript-Syntaxprüfung. Der Browser-Test meldet sich mit `admin` an, öffnet alle Ansichten, ergänzt weitere Zeitkarten, prüft Sekundenfeld und Checkbox-basierte Automatenauswahl, bestätigt die einklappbare Zusammenfassung und prüft die Ereigniskarten im Verlauf. Die Navigation wird bei Smartphone-Breite ohne horizontale Tab-Scrollleiste dargestellt.

## Ergänzung v1.6

ESP32- und ESP8266-Firmware wurden erfolgreich kompiliert; beide LittleFS-Images wurden erzeugt. Die nativen Controller-/Zeitplantests bestanden nach Ergänzung der Sekundenfelder. Der Browsertest meldet sich mit `admin` an, öffnet alle sieben Ansichten, ergänzt mehr als acht Zeitplanzeilen, prüft `step=1` für Sekunden, Automatenauswahl, fehlendes Dauerfeld und die Darstellung des Verlaufs als Ereigniskarten statt JSON.

Noch offen bleibt ein realer Mehrgeräteversuch mit mehreren ESPs: sekundengenau versetzte Motorstarts, WLAN-Unterbrechung während der Verteilung und die RTC-Abweichung zwischen Geräten müssen auf Hardware gemessen werden. Für zuverlässiges Versetzen sollten alle Automaten regelmäßig per NTP synchronisiert sein und einige Sekunden Abstand erhalten.

## Ergänzung v1.5

Der Browsertest prüft nicht mehr nur vorhandene Überschriften, sondern die aktive Sichtbarkeit jeder einzelnen Ansicht. Nach Anmeldung mit `admin` wurden Übersicht, Zeitplan, Futter & Kalibrierung, Automaten, Einstellungen, Backup und Verlauf erfolgreich durchgeklickt. Der Zeitplan zeigte acht Karten, Futtersortenauswahl und kein Dauerfeld. Die zuvor blockierende JavaScript-Ausnahme wurde damit reproduziert und nach der Korrektur ausgeschlossen.

## Ergänzung v1.4

ESP32- und ESP8266-Firmware sowie beide LittleFS-Images wurden erfolgreich gebaut. Die 64 nativen Controller-Assertions bestanden weiterhin. JavaScript von Produktoberfläche und Demo bestand die Syntaxprüfung. Die eigenständige Demo wurde zusätzlich automatisiert in Microsoft Edge mit `admin` angemeldet: alle sieben Tabbezeichnungen, acht Zeitplan-Karten, fehlendes Dauerfeld, Futtersortenauswahl, Kalibrierungs- und Automatenansicht, Status „Alles okay“ und Versionsanzeige v1.4 wurden geprüft.

Nicht mit realen Geräten integriert getestet wurden Browser-CORS über mehrere IP-Adressen, Gruppenschlüssel, Teilausfall mehrerer Automaten, gleichzeitige Motorstarts und Zeitplanübertragung. Vor dem Einsatz mindestens zwei Automaten im abgeschotteten WLAN koppeln, verschiedene Kalibrierungen unter demselben Futternamen prüfen und Netzwerkunterbrechungen während Gruppenaktionen provozieren.

## Ergänzung v1.3

Die Demo-JavaScript-Syntax wurde geprüft. Zusätzlich wurde `demo.html` in Microsoft Edge geladen, mit `admin` angemeldet und danach geprüft, dass Navigation, Übersicht, Statuswert „Motor: Aus“ und Abmelden sichtbar sind. Beide produktiven LittleFS-Images wurden nach der Versionsanzeige erneut gebaut. Die übrige Demo-Interaktion ist simuliert und ersetzt keine Hardwareprüfung.

## Ergänzung v1.2

Erneut geprüft: Firmware-Builds für ESP32-WROOM/DevKit und ESP8266 D1 mini, LittleFS-Images und JavaScript-Syntax. Die nativen C++-Tests enthalten jetzt **64 Assertions**, darunter Proportionalberechnung nach Zeit und Umdrehungen, Grenzfälle, einen portionierten Zeitplanslot und ein fehlendes Futterprofil. Hardware, Browser mit echten Geräten, Stromausfall während Profilspeicherung, Backup-Download sowie die eingebettete Mehrgeräteansicht wurden nicht integriert getestet. Die Grammportion ist eine Schätzung aus extern gewogenem Probelauf.

## Ergänzung v1.1

Nach Smartphone-Formularen und Wiederherstellungscode erneut geprüft: ESP32- und ESP8266-Firmware-Build erfolgreich, JavaScript-Syntaxprüfung bestanden, beide LittleFS-Images gebaut. Die 54 vorhandenen Controller-Assertions betreffen unveränderte Motor- und Zeitplanlogik. Der neue Wiederherstellungsablauf wurde nicht mit einem angeschlossenen Gerät oder echtem HTTP-Client geprüft. Dafür sind insbesondere falsche Codes, fünfter Fehlversuch, Codeverbrauch, Neustart, Dateisystemfehler beim Passwortwechsel und ungültige Passwortlängen auf Hardware zu testen.

## Umfang

Bearbeitet wurde die tatsächlich bereitgestellte FeedMate-main.zip, nicht nur der öffentliche GitHub-Stand. Ursprüngliche Dateien, Lizenz und historische Varianten bleiben enthalten. Aktive Firmware, Buildkonfiguration, Weboberfläche, Tests und Dokumentation wurden überarbeitet. Auf Wunsch ist ESP32 jetzt Standardziel; ESP8266 bleibt unterstützt.

Die Prüfungen liefen auf Windows ohne angeschlossene FeedMate-Hardware. Eine Buildfreigabe ist keine Hardwarefreigabe.

## Ausgeführte Prüfungen

| Prüfung | Ergebnis |
|---|---|
| ESP32-WROOM/DevKit, `pio run -e esp32dev` | Erfolgreich kompiliert und gelinkt, Firmware-Image erzeugt |
| ESP8266 D1 mini, `pio run -e d1_mini` | Erfolgreich kompiliert und gelinkt, Firmware-Image erzeugt |
| C++17-Tests der Produktionslogik | 64 Assertions bestanden (v1.2) |
| JavaScript der ausgelieferten Weboberfläche | Syntaxprüfung mit Node.js bestanden |

Dateisystem-Builds und endgültige Speicherwerte werden in `validation/SUMMARY.txt` ausgewiesen. Die Buildprotokolle liegen ebenfalls unter `validation/`.

Verwendet: PlatformIO Core 6.2.0, Espressif32 6.10.0 / Arduino 2.0.17 / Xtensa GCC 8.4.0, Espressif8266 4.2.1 / Arduino 3.1.2 / Xtensa GCC 10.3.0. ArduinoJson 6.21.5, PubSubClient 2.8.0, RTClib 2.1.4, Adafruit BusIO 1.16.1. Native Tests: Zig 0.13.0, C++17. Die Assertions waren aktiviert.

## Was die 64 Assertions abdecken

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
