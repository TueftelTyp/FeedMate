# FeedMate v1.6

## Zeitplan

- Uhrzeit im Format `HH:MM:SS`
- bis zu 32 gespeicherte Zeiten je Automat
- bis zu 96 Zeilen im gemeinsamen Smartphone-Plan
- Automat, Futtersorte und Grammportion je Zeile
- neue Zeilen über **Weitere Fütterungszeit**
- gemeinsames Speichern und Verteilen an alle betroffenen Automaten

Mehrere Automaten sollten um einige Sekunden versetzt werden. Ihre Uhren müssen dafür gültig und möglichst per NTP synchronisiert sein.

## Verlauf

Der Verlauf zeigt keine JSON-Struktur mehr. Ereignisse erscheinen als Karten mit Symbol, deutschem Titel, Zeitpunkt und lesbarem Detailtext. Warnungen und Fehler sind farblich hervorgehoben.

## Speicherung

Jeder Automat speichert seine eigenen zugewiesenen Zeiten dauerhaft. Die übergreifende Zuordnung „Zeit → Automat“ liegt im Browser-Speicher des verwaltenden Smartphones. Beim Löschen der Browserdaten bleiben die Gerätepläne erhalten, aber die zentrale Zuordnung muss neu geladen beziehungsweise aufgebaut werden.
