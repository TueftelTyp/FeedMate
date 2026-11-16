# FeedMate v1.2 – Aktualisierung von v1.1

Dieses Änderungs-ZIP enthält ausschließlich neue oder veränderte Dateien **gegenüber v1.1**. Die Dateien im bestehenden v1.1-Projekt ersetzen. Danach Firmware und LittleFS-Image für das passende Board erneut bauen und übertragen. Vor `uploadfs` die bestehenden LittleFS-Daten sichern: `uploadfs` ersetzt sonst Konfiguration, Zeitplan, Logs und Futterprofile.

## Bedienung

1. Gerät öffnen und anmelden. Vorher sind nur Login und Wiederherstellung zu sehen.
2. Unter **Übersicht** einen kurzen Probelauf auslösen. Futter extern wiegen; bei aktivem Hall-Sensor die gemeldeten Umdrehungen ablesen.
3. Unter **Portionen** Name, Steuerungsart (Zeit oder Hall-Umdrehungen), Probelaufzeit, gemessene Gramm und Umdrehungen eintragen. Profil speichern. Dann Futtersorte wählen und gewünschte Grammzahl eingeben.
4. Im **Zeitplan** optional für einen Slot Futtersorte und Gramm auswählen. Ein Slot ohne Sorte bleibt ein reiner Zeitlauf.
5. Unter **Geräte** weitere FeedMate-Webadressen hinzufügen. Sie erscheinen auf diesem Smartphone als eingebettete Einzeloberflächen; jede benötigt eigene Zugangsdaten. Browser-Speicher löschen entfernt die gespeicherte Liste.
6. Unter **Backup** die Einstellungen als JSON-Datei exportieren. Sie enthält Geheimnisse und muss vertraulich aufbewahrt werden. Ein Import-Assistent ist noch nicht vorhanden.

Die Grammangabe ist eine proportionale Schätzung aus dem Probelauf, kein Messwert der tatsächlichen Ausgabe. Mechanik, Futtersorte und Füllstand können die Menge verändern. Grenzwerte von 30 Sekunden und konfigurierten Höchstdrehungen bleiben wirksam. Ein fehlendes/ungeeignetes Profil im Zeitplan führt zu einem protokollierten, nicht automatisch wiederholten Fehlversuch.

## Technische Grenzen

Die Geräteansicht ist eine browserbasierte Auswahl, keine gemeinsame Firmware-Steuerung. Es findet keine automatische Synchronisierung von Zeitplänen oder Profilen statt. Browser oder Geräte können die Einbettung fremder Seiten blockieren; dann die Geräteadresse separat öffnen. Firmware-Builds und Logiktests sind dokumentiert, aber die Dosierung und die Mehrgeräteansicht benötigen Tests an echten Geräten.
