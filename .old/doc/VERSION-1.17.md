# FeedMate v1.17

v1.17 korrigiert das SHA-256-Passworthashing für mbedTLS 3 in Arduino-ESP32 3.3.12. Dadurch kompiliert der Webserver nun für ESP32-C3, C6, S2, S3 und den klassischen ESP32 mit der in `platformio.ini` festgeschriebenen Plattform. Die Kompatibilität mit mbedTLS 2 bleibt erhalten. Alle Boardprofile, Smartphone-Oberflächen und Funktionen aus v1.16 sind unverändert enthalten.
