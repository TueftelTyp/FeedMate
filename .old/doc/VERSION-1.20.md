# FeedMate v1.20

v1.20 aktiviert die native USB-Konsole für ESP32-C3, C6, S2 und S3. Das Setup-WLAN wird nicht mehr ungeprüft nur einmal gestartet: FeedMate wertet den Start aus, meldet Status und IP, versucht einen fehlgeschlagenen Start alle fünf Sekunden erneut und wiederholt den aktiven AP-Status alle 30 Sekunden. Die festen Erstzugänge bleiben `feedmate` für das Setup-WLAN und `FeedMateAdmin` für die Verwaltung; die Demo bleibt bei `admin`. Alle Boardprofile und Flashkorrekturen aus v1.19 sind enthalten.
