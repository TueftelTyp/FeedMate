# ESP32 und ESP8266

Der Standard-Build ist **ESP32 DevKit / ESP32-WROOM-32** (`esp32dev`, 4 MB Flash, 240 MHz). Der **WeMos D1 mini / ESP8266** bleibt als `d1_mini` unterstützt. Beide verwenden dieselben Fütterungs-, Authentifizierungs-, Speicher- und Sensorabläufe.

## Ziel auswählen

```sh
# ESP32, Standard
pio run -e esp32dev
pio run -e esp32dev -t buildfs
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs

# ESP8266
pio run -e d1_mini
pio run -e d1_mini -t buildfs
pio run -e d1_mini -t upload
pio run -e d1_mini -t uploadfs
```

Die Images sind nicht zwischen den Plattformen austauschbar. `uploadfs` ersetzt die persistenten Daten; Migrationshinweise in HowToFlash.md beachten. Ein Wechsel auf ein anderes Gerät ändert außerdem die geräteabhängige Passwort-Salzung und MQTT-ID. Deshalb dort neu einrichten und keine alten Passworthashes übernehmen. Zeitpläne nach der Übernahme bewusst prüfen; eine alte Reservierung nicht versehentlich löschen, wenn am selben Tag bereits gefüttert wurde.

## Anschlüsse

| Funktion | ESP32-WROOM / DevKit | ESP8266 D1 mini |
|---|---:|---:|
| Motor-MOSFET Gate | GPIO25 | D5 / GPIO14 |
| Jam-Eingang, HIGH = Fehler | GPIO26 | D6 / GPIO12 |
| Lichtschranke, HIGH = Futter | GPIO27 | D7 / GPIO13 |
| Hall, fallende Flanke | GPIO33 | D8 / GPIO15 |
| Taster gegen GND | GPIO32 | D3 / GPIO0 |
| DS3231 SDA | GPIO21 | D2 / GPIO4 |
| DS3231 SCL | GPIO22 | D1 / GPIO5 |
| Versorgungserkennung | GPIO34 / ADC1 | A0 |
| Status-LED | GPIO2 | LED_BUILTIN |

Die Signal-Pins des ESP32 vermeiden für Motor, Hall und Taster die Boot-Strapping-Pins. GPIO34 ist nur Eingang und hat keine internen Pull-Widerstände. Der ADC1-Kanal funktioniert zusammen mit WLAN; ADC2 wurde bewusst vermieden. Beide Plattformen arbeiten mit 3,3-V-Logik. Den Spannungsbereich am ADC anhand des konkreten Boards und Spannungsteilers prüfen. Die ADC-Schwellen 450/550 beziehen sich auf 10 Bit und sind **keine Volt- oder Batterieprozentangabe**. Beim ESP32 wird die 10-Bit-Auflösung explizit eingestellt; 11-dB-Dämpfung ersetzt keine Kalibrierung.

Pinbelegung steht zentral in `code/platform.h`. ESP32-C3, S2, S3, C6 sowie Boards mit anderer Verdrahtung sind **keine geprüften Drop-in-Ziele**; sie benötigen ein eigenes Pin-/Boardprofil. Diese ZIP behauptet nur Builds für den klassischen ESP32 und ESP8266.

Der obligatorische Jam-Eingang erhält einen internen Pull-up: offen/abgezogen bedeutet HIGH und sperrt den Start. Der Sensor muss im gesunden Zustand LOW liefern. Optionale Hall-Interrupts werden nur bei aktivierter Hall-Überwachung zugeschaltet; unbeschaltete optionale Eingänge erzeugen dadurch keine Interruptlast. Hall- und Lichtschrankenausgänge benötigen passende externe Pegel/Pull-Widerstände; ein digitaler Eingang kann nicht jeden Kabelbruch erkennen. Die ESP32-Status-LED ist aktiv HIGH, die D1-mini-LED aktiv LOW.

## ESP32-Anpassungen

- 240-MHz-Build und größere Speicherreserven; dynamische JSON-Dokumente statt großer lokaler Stackobjekte.
- Native WiFi/WebServer-Anbindung, mbedTLS SHA-256 und Hardware-Zufallsquelle. WLAN wird vor der ersten Zufallszahlerzeugung aktiviert, um die RF-Entropiequelle verfügbar zu machen.
- Native LittleFS- und Zeitsynchronisierungs-APIs; Formatierung bei Mountfehler bleibt deaktiviert.
- Plattform-ID, eindeutige Geräte-ID und freier Heap in `/api/status` für Diagnose.
- FreeRTOS-gerechte kurze Pause während des Motorlaufs; Hall-Zählung per Interrupt auf beiden Plattformen.
- Gemeinsame, sequentielle Zustandsverwaltung. Die beiden CPU-Kerne werden nicht durch konkurrierende Anwendungstasks auf Motor und Speicher zugreifen gelassen. Web und MQTT pausieren weiterhin während der Fütterung, damit Netzwerkwartezeiten keine Motorabschaltung verschleppen. Die Mehrleistung wird nicht als Echtzeitgarantie ausgegeben.

Referenzen: [Espressif ADC API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html), [Espressif RNG](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/random.html). Die konkret gebauten Versionen stehen im Testbericht.
