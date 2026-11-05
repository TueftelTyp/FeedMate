# FeedMate v1.3 – Aktualisierung von v1.2

Dieses ZIP enthält nur die Änderungen gegenüber v1.2. Es ergänzt eine eigenständige `demo.html` und aktualisiert die Versionsanzeige sowie Dokumentation.

## Demo verwenden

1. `demo.html` aus dem ZIP an einen beliebigen Ort kopieren.
2. Die Datei direkt im Browser öffnen; ein Webserver ist nicht erforderlich.
3. Mit dem Kennwort **admin** anmelden.
4. Status, Probelauf, Futtersorten, Portionen, Zeitplan, Konfiguration, Geräteauswahl, Verlauf und Backup ausprobieren.

Die Demo speichert Änderungen im lokalen Browser-Speicher. Sie greift nicht auf einen Motor oder FeedMate zu. Der Demo-Wiederherstellungscode lautet `demo`; nach einer simulierten Änderung bleibt das Kennwort `admin`. Die Geräteansicht kann echte Adressen öffnen, wenn du dort welche einträgst. Das ist dann kein simulierter Zugriff mehr und erfordert die Anmeldung des jeweiligen Geräts.

Für die echte Geräteoberfläche bleibt `data/index.html` zuständig. `demo.html` wird nicht in das LittleFS-Image aufgenommen und verbraucht daher keinen Flash-Speicher des Controllers.
