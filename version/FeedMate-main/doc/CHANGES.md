# Änderungen vom 05.11.2026

## v1.15 – Bereinigtes Release-Paket

- Historischen Ordner `code/old` aus dem Release entfernt.
- Frühere `INSTALLATION-v*.md`- und `VERSION-*.md`-Dateien aus dem Release entfernt.
- Nur die aktuelle Installationsanleitung und Versionsübersicht beigelegt.
- Aktuellen Quellcode, Plattformkonfiguration, Dokumentation und Prüfprogramme beibehalten.

## v1.14 – Kompakte Entfernen-Symbole

- Große „Entfernen“-Schaltflächen in der Automatenansicht durch Mülleimer-Symbole ersetzt.
- Gerätekarte für Desktop und Smartphone neu ausgerichtet.
- Zugängliche Beschriftung und Tooltip für die Löschaktion ergänzt.

## v1.13 – Einheitliche Info- und Aktualisierungsbedienung

- Leere weiße Anmeldekachel nach erfolgreichem Login ausdrücklich ausgeblendet.
- Übersicht, Zeitplan, Einstellungen, Backup, Kalibrierung, Automaten und Verlauf besitzen einheitliche Titelzeilen.
- Statische Erklärungstexte in ausblendbare Info-Popups verschoben.
- Breite „Neu laden“-/„Aktualisieren“-/„Status laden“-Buttons durch kompakte Aktualisieren-Icons rechts im Kacheltitel ersetzt.
- Rückmeldungen wie „Erledigt.“ und Fehlermeldungen werden nach 4,5 Sekunden automatisch entfernt; laufende Probelaufmeldung bleibt bis zum Ergebnis sichtbar.
- Demo-Simulation zusätzlicher Automaten bleibt erhalten; Produkt-API unverändert.
- Vollständiges Projekt und aktualisierte Installationsanleitung ausgeliefert.

## v1.12 – Demo-Automaten ohne Hardware

- Nur `demo.html`: Hinzugefügte Automaten benötigen kein erreichbares Gerät und keinen HTTP-Endpunkt.
- Demo simuliert Remote-Status, Gruppenportionen und Zeitplanübertragung; die Geräte erscheinen anschließend auch in der Automatenauswahl des Zeitplans.
- Produktive `data/index.html`, Firmware und echte Flottenprüfung bleiben unverändert.
- Vollständiges Projekt-ZIP statt eines reinen Änderungspakets.
- Neue ausführliche Installationsanleitung für ESP32 und ESP8266.

## v1.11 – Aktive/inaktive Zeiten und korrigierte Feldbreite

- Aktive Zeitkarten grün, inaktive rot markiert; Nummernmarke und Rahmen folgen dem Zustand sofort beim Umschalten.
- Großen Löschbutton entfernt und durch einen Mülleimer auf derselben Zeile wie „Aktiv“ ersetzt.
- Editor, Grid-Elemente, Eingaben, Selects und Automaten-Fieldset erhalten `min-width: 0` und eine maximale Breite von 100 Prozent.
- Mobile Innenabstände vereinheitlicht; kein Abschneiden mehr am rechten Kartenrand.
- Gemeinsamer Smartphone-Plan verwendet ab v1.11 einen neuen Browser-Schlüssel und übernimmt ältere v1.7/v1.6-Daten weiterhin.

## v1.10 – Zeitplan-Layout neu aufgebaut

- Globalen Hilfe-Block entfernt. Zeitplan-Hilfe sitzt als Icon direkt neben „Gemeinsamer Zeitplan“ und öffnet den Erklärungstext als Popup.
- Aktualisieren sitzt als kleiner Iconbutton rechts daneben; kein breiter „Neu laden“-Button mehr.
- Eingeklappte Fütterungen neu gestaltet: erste Zeile Uhrzeit und Portion, zweite Zeile ausgewählte Automaten, separater Aufklapp-Pfeil.
- „Fütterung 1“/„Details“ aus der gequetschten Textzeile entfernt; nur eine kleine Nummernmarke bleibt.
- Auf Smartphones sind Aktivierung, Uhrzeit, Automatenauswahl, Futtersorte und Portion im Editor jeweils vollbreit untereinander angeordnet.
- Leeren Anmeldecontainer nach dem Login ausgeblendet.
- Demo zeigt `admin` im Passwort-Platzhalter und verwendet ansonsten dieselbe Oberfläche.

## v1.9 – Header, Hilfe und kompakte Aktualisierung

- Sticky-Header entfernt; der Kopfbereich scrollt wieder normal mit der Seite.
- Vollständige Statusanzeige mit Lampe und Text wiederhergestellt.
- Abmelden sitzt nach der Anmeldung rechts im Header neben der Statusanzeige.
- Statischer Bedienhinweis durch eine aufklappbare Hilfe-Schaltfläche ersetzt.
- Demo-Hinweis bleibt als einzige bewusste Demo-Abweichung erhalten.
- „Neu laden“ als kompakter Button statt als breite Hauptaktion gestaltet.

## v1.8 – Burger-Menü, sichtbarer Header und vereinfachter Verlauf

- Mobile Navigation durch ein Burger-Menü ersetzt; die Tab-Leiste wird erst nach „Menü“ geöffnet und schließt nach der Auswahl automatisch.
- Header mit Statuslampe bleibt beim Scrollen sticky sichtbar.
- Zeitkarten-Zusammenfassung enthält nur noch Uhrzeit, Automaten und Grammportion. Futtersorte und weitere Felder bleiben im ausgeklappten Bereich.
- Verlauf auf kompakte Ereigniszeilen mit Symbol, Titel, Zeitpunkt und Detailtext reduziert.
- Demo und produktive Oberfläche bleiben aus derselben `data/index.html` abgeleitet.

## v1.7 – Mehrfachautomaten und kompakte Zeitkarten

- Jede gemeinsame Fütterungszeit kann mehreren Automaten gleichzeitig zugewiesen werden; beim Verteilen wird der Eintrag je Zielautomat berücksichtigt.
- Einzelne Zeitkarten sind standardmäßig eingeklappt. Die Zusammenfassung zeigt nur Uhrzeit, ausgewählte Automaten, Grammportion und Futtersorte. Im aufgeklappten Editor bleiben Aktivierung, `HH:MM:SS`, Mehrfachauswahl, Futtersorte und Gramm konfigurierbar.
- Die Automaten werden als Checkbox-Gruppe dargestellt, damit mehrere Ziele ohne unübersichtliches Mehrfach-Select markiert werden können.
- Die obere Smartphone-Navigation verwendet Zeilenumbruch statt horizontaler Scrollleiste.
- Demo und produktive `data/index.html` teilen weiterhin dieselbe Logik; die Demo ergänzt nur Kennwort, Hinweis und Mock-API.

## v1.6 – Sekundengenauer Gruppenzeitplan und verständlicher Verlauf

- Zeitplanslots besitzen jetzt Stunde, Minute und Sekunde. API, LittleFS-Speicherung und Ausführung berücksichtigen `second`; bestehende Pläne werden mit Sekunde `00` übernommen.
- Kapazität von acht auf 32 Zeitpunkte je Automat erhöht. Die API nimmt bis zu 32 Einträge an und füllt nicht verwendete Plätze deaktiviert auf.
- Der Smartphone-Gruppenplan erlaubt bis zu 96 Zeilen, freie Ergänzung und Entfernung sowie eine Automatenauswahl pro Zeile. Beim Speichern werden die Einträge nach Automat getrennt verteilt.
- Teilfehler bei der Verteilung nennen die nicht aktualisierten Automaten; erfolgreiche Ziele bleiben gespeichert.
- Verlauf ersetzt JSON-Rohdaten durch Ereigniskarten mit Symbol, verständlichem deutschen Titel, Zeitpunkt, Detail und Warn-/Fehlerfarbe.
- Smartphone-Layout: kompaktere Kartenköpfe, 16-Pixel-Formulare gegen Browser-Zoom, klarere Abstände und vollbreite Aktionen auf kleinen Displays.

## v1.5 – Navigation repariert

- Veralteten Handler für den entfernten manuellen Dauer-Button gelöscht. Dieser JavaScript-Fehler hatte die Registrierung der Tab-Navigation verhindert.
- Ansichten werden über eine explizite `active`-Klasse statt nur über das `hidden`-Attribut umgeschaltet.
- Demo-Kopien verwenden eine JSON-kompatible Kopie statt `structuredClone` und funktionieren dadurch auch mit älteren Smartphone-Browsern.
- Browserprüfung erweitert: Jeder Tab muss nach dem Anklicken tatsächlich sichtbar sein; der Zeitplan muss acht Karten anzeigen.

## v1.4 – Klare Smartphone-Verwaltung und Automaten-Gruppe

- Navigation und Inhalte sind eindeutig zugeordnet: Übersicht, Zeitplan, Futter & Kalibrierung, Automaten, Einstellungen, Backup und Verlauf.
- Die Statuslampe im Kopf fasst Motorstörung, Speicher, Uhr, RTC, Futterstand und den zuletzt geladenen Gruppenstatus zusammen.
- Kalibrierung als sichtbarer Dreischritt: Futtersorte/Modus wählen, Motor-Probelauf starten und Hall-Umdrehungen ablesen, extern gewogene Gramm eintragen und Profil speichern.
- Zeitplan enthält keine manuelle Dauer mehr. Aktive Einträge benötigen Futtersorte und Gramm; die Firmware berechnet Laufzeit beziehungsweise Zielumdrehungen aus dem lokalen Profil.
- Mehrere Automaten werden per Gruppen-API direkt vom Smartphone angesprochen. Gemeinsame Portion und explizite Zeitplanübertragung laufen für alle ausgewählten Ziele; Teilerfolge und nicht erreichbare Geräte werden gemeldet.
- Der neue schreibgeschützte Gruppenschlüssel wird als gerätespezifisch gesalzener Hash gespeichert. Remote-Aufrufe verwenden `X-FeedMate-Key`; Status/config geben das Geheimnis nie zurück. CORS und OPTIONS sind für die direkte lokale Smartphone-Kommunikation ergänzt.
- `POST /api/portion` akzeptiert zusätzlich `foodName`, damit gleich benannte, aber je Automat separat kalibrierte Profile gemeinsam dosiert werden können.
- Gerätename und Gruppenaktivierung sind konfigurierbar. Bis zu zwölf entfernte Automaten werden ausschließlich im Browser-Speicher des Smartphones gespeichert.
- Sicherheitsgrenze: HTTP und der im Browser gespeicherte Gruppenschlüssel sind nur für ein vertrauenswürdiges LAN vorgesehen. Keine Portfreigabe ins Internet.

## v1.3 – Eigenständige Demo

- `demo.html` simuliert die vollständige Web-API im Browser und benötigt weder Firmware noch Netzwerkgerät.
- Anmeldung immer mit `admin`. Simulierte Passwortänderungen verändern dieses Demo-Kennwort nicht; Wiederherstellungscode der Demo ist `demo`.
- Demo-Daten bleiben im lokalen Browser-Speicher. Status, Probelauf, Portionen, Futtersorten, Zeitplan, Einstellungen, Verlauf und JSON-Backup sind interaktiv.
- Die produktive Oberfläche zeigt v1.3 an. Der Firmwarecode für Motor, Zeitplan und Authentifizierung ist gegenüber v1.2 unverändert.

## v1.2 – Portionen und Geräteansicht

- Verwaltungsbereiche bleiben vor dem Login verborgen; API-Zugriffe waren bereits geschützt. Der statische HTML-Quelltext ist weiterhin abrufbar, enthält aber keine Gerätedaten.
- Bis zu acht Futtersorten werden atomar in `/foods.json` gespeichert. Jede enthält Name, Probelaufzeit, extern gemessene Gramm, optionale Hall-Umdrehungen und Zeit- oder Umdrehungsmodus. Namen sind eindeutig.
- `POST /api/portion` nimmt Profilindex und gewünschte Gramm an. Die Produktionslogik rechnet proportional und rundet Zeit bzw. Drehungen auf. Maximum 30 Sekunden und konfigurierte Höchstdrehzahl bleiben wirksam. Keine echte Gewichtsmessung oder automatische Anpassung.
- Zeitplanslots können optional Futtersorte und Gramm enthalten. Bestehende reine Laufzeit-Slots bleiben gültig. Reservierung vor Motorstart und höchstens ein Versuch pro Tag gelten weiter. Aktive Slots blockieren das Löschen ihrer Futtersorte.
- Authentifizierte Endpunkte `GET/POST /api/foods`, `POST /api/portion`, `GET /api/backup`. Der Export enthält Konfiguration einschließlich Geheimnissen, Zeitplan samt Reservierungsdaten und Futtersorten. Noch kein Import.
- Browserbasierte Geräteauswahl speichert Name und Adresse lokal auf dem Smartphone und zeigt weitere Geräte in einem eingebetteten Fenster. Jedes Gerät verlangt seine eigene Anmeldung. Es gibt keine automatische Kopplung auf Firmwareebene, gemeinsame Zeitpläne oder Flottenauthentifizierung.
- Die Smartphone-Oberfläche zeigt nach Anmeldung Status, Kalibrierung, Zeitplan, Einstellungen und Export. Vor Anmeldung sind diese Bereiche verborgen.

## v1.1 – Smartphone-Konfiguration und Passwortrettung

- Bedienoberfläche auf kleine Bildschirme ausgelegt: große Bedienelemente, Statuskacheln, Zeitplanfelder in Sekunden und Eingabefelder für alle aktiv genutzten Einstellungen (WLAN, MQTT, Zeitzone, Jam-Zeit und optionale Sensoren). Die Einstellungen erfordern keine JSON-Bearbeitung mehr.
- Das bisherige Admin-Passwort bleibt geheim. Bei jedem Boot wird über USB/Serial ein zufälliger 128-Bit-Wiederherstellungscode gezeigt. `POST /api/recover` nimmt `{\"code\":\"...\",\"newPassword\":\"...\"}` an, schreibt den neuen Passworthash atomar und invalidiert die Sitzung. Ein erfolgreicher Code ist einmalig; fünf Fehlversuche pro Boot und zwei Sekunden Abstand begrenzen Online-Versuche. Physischer Zugriff auf USB/Serial ist für die Codekenntnis nötig.
- Firmware für ESP32 und ESP8266 nach den Änderungen erneut gebaut; Web-JavaScript und beide LittleFS-Images geprüft. Reale Geräteprüfung steht weiter aus.

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
