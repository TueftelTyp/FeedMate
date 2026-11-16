# FeedMate v1.23

v1.23 ersetzt die freie POSIX-Zeitzoneneingabe durch eine smartphonegerechte,
gruppierte Auswahlliste mit Namen wie `Europe/Berlin`. Die Firmware übersetzt
diese Namen intern; alte gespeicherte POSIX-Werte bleiben kompatibel.

Das FeedMate-Verwaltungs-AP bleibt nun auch dann aktiv, wenn der Automat mit
dem gespeicherten Heimnetz verbunden ist. Der Gerätename dient nach einem
Neustart als bereinigter Netzwerk-Hostname. Die AP-SSID bleibt davon unabhängig
`FeedMate-<Geräte-ID>`, damit sie nach Umbenennungen weiterhin auffindbar ist.
