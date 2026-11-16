# FeedMate v1.18

v1.18 verwendet auf allen ESP32-, C3-, C6-, S2- und S3-Zielen das `huge_app.csv`-Partitionsschema. Damit stehen 3.145.728 Byte für die Firmware und 917.504 Byte für LittleFS zur Verfügung. Das in v1.17 korrigierte ESP32-C3-Abbild mit 1.399.637 Byte passt nun mit großer Reserve. Das Schema besitzt keine zweite OTA-Partition; FeedMate-Aktualisierungen erfolgen weiterhin per USB. ESP8266 bleibt unverändert.
