# FeedMate - Complete Build Guide
## ESP8266 WiFi-Enabled Automatic Pet Feeder with RTC & Jam Protection

---

## Table of Contents

1. [German Version (Deutsch)](#german-version)
   - [Stückliste](#stückliste)
   - [Verdrahtungsplan](#verdrahtungsplan)
   - [Aufbau-Tipps](#aufbau-tipps)
   - [Einkaufsliste](#einkaufsliste)

2. [English Version](#english-version)
   - [Parts List](#parts-list)
   - [Wiring Overview](#wiring-overview)
   - [Build Tips](#build-tips)
   - [Shopping List](#shopping-list)

---

## German Version

### Stückliste

#### Mikrocontroller & Zeitgeber
- **1x Wemos D1 Mini** (ESP8266)
- **1x DS3231 RTC Modul** (mit I2C, NICHT DS1307!)

#### Motorsteuerung
- **1x IRLZ44N oder IRLB8721 MOSFET** (Logic-Level N-Channel)
- **1x Freilaufdiode 1N4007** (oder 1N5408 bei größeren Motoren)
- **1x 10kΩ Widerstand** (Pull-Down für MOSFET Gate)

#### Klemmschutz (Hardware-LED)
- **1x LM393 Komparator** (Dual Op-Amp)
- **1x 0,1Ω Shunt-Widerstand** (2-5W, präzise)
- **1x LED** (rot, 5mm)
- **1x 220Ω Widerstand** (LED Vorwiderstand)
- **1x 10kΩ Potentiometer** (Referenzspannung einstellen)
- **1x 1kΩ Widerstand** (für Potentiometer)

#### Stromversorgung
- **2x 18650 Li-Ion Akku** (z.B. Panasonic NCR18650B, je ~3000mAh)
- **1x 18650 Dual Akkuhalter** (mit Kabeln, 2P parallel)
- **1x TP4056 Lademodul** (MIT Schutzschaltung DW01+8205A!)
- **1x MT3608 Step-Up Wandler** (auf 5V einstellbar)
- **1x 1000µF Elko** (10V, für Pufferung)

#### Bedienelemente
- **1x Taster** (NO - normally open, 12mm oder größer)
- **1x 10kΩ Widerstand** (Pull-Up für Taster)

#### Sonstiges
- **1x 4,7kΩ Widerstand** (I2C Pull-Up, falls RTC-Modul keine hat)
- **1x 100nF Keramik-Kondensator** (Entstörung)
- **Diverse Jumper-Kabel, Litze, Schrumpfschlauch**
- **1x PCB/Lochrasterplatine oder eigene Platine**

---

### Verdrahtungsplan

#### 1. Stromversorgung (5V Hauptbus)

```
TP4056 OUT+ ──────────────┬────────────── MT3608 IN+
                          │
TP4056 OUT- ─────────────────────────── MT3608 IN-

MT3608 OUT+ (5V) ─────┬───────────────── +5V Hauptbus
                      │
MT3608 OUT- (GND) ───────────────────── GND Hauptbus

+5V Hauptbus geht zu:
─ D1 Mini VCC
├─ DS3231 VCC
├─ Motor-Treiber VCC
─ LM393 VCC (Pin 8)
└─ Taster (eine Seite)

GND Hauptbus geht zu:
─ D1 Mini GND
├─ DS3231 GND
├─ Motor-Treiber GND
├─ LM393 GND (Pin 4)
├─ MOSFET Source
├─ Shunt-Widerstand (eine Seite)
└─ Taster (über Pull-Up)
```

#### 2. RTC (DS3231)

```
DS3231 VCC  ───────────── +5V
DS3231 GND  ───────────── GND
DS3231 SDA  ───────────── D1 Mini D2 (GPIO4)
DS3231 SCL  ──────────── D1 Mini D1 (GPIO5)
DS3231 SQW  ───────────── D1 Mini D0 (GPIO16) [für DeepSleep Wake-Up]
```

**Wichtig:** DS3231 braucht KEINE separate Knopfzelle! Die Backup-Stromversorgung läuft über den 18650 Akku.

#### 3. Motorsteuerung

```
D1 Mini D5 (GPIO14) ──┬── 10kΩ ── GND
                      │
                      ── MOSFET Gate

MOSFET Drain ─────────┬── Motor (-)
                      │
                      └── 1N4007 Kathode (Strich)

Motor (+) ──────────── +5V

1N4007 Anode ───────── Motor (-)
1N4007 Kathode ──────── Motor (+)

MOSFET Source ───────── GND
```

**Motor-Anschluss:**
- Motor(+) direkt an +5V
- Motor(-) an MOSFET Drain
- Freilaufdiode quer über den Motor (Kathode an +5V, Anode an Motor-)

#### 4. Klemmschutz mit Hardware-LED

```
Motor (+) ─────────────┬───── Motor-Treiber/MOSFET
                       │
                      [0,1Ω Shunt]
                       │
                       ├───── GND
                       │
LM393 Pin 3 (+) ───────┤
                       │
LM393 Pin 2 (-) ───────── 10kΩ Poti (Schleifer)
                       │
10kΩ Poti Ende 1 ──────┴── +5V (über 1kΩ)
10kΩ Poti Ende 2 ──────── GND

LM393 Pin 8 ───────────── +5V
LM393 Pin 4 ───────────── GND
LM393 Pin 7 (Output) ────┬── 220Ω ── LED(+)
                         │
                         ── D1 Mini D6 (GPIO12) [für Software-Logging]

LED(-) ────────────────── GND
```

**Funktionsweise:**
- Der Shunt-Widerstand erzeugt eine Spannung proportional zum Motorstrom
- Bei 800mA: 0,8A × 0,1Ω = 0,08V (80mV)
- Das Potentiometer stellt die Referenzspannung ein (z.B. 0,1V = 1A)
- Wenn Motorstrom > Schwellwert → LM393 Output HIGH → LED leuchtet

#### 5. Manueller Taster

```
D1 Mini D7 (GPIO13) ──┬── Taster ── GND
                      │
                      └── 10kΩ ── +5V (Pull-Up)
```

**Funktionsweise:**
- Normal: Pin ist HIGH (durch Pull-Up)
- Taster gedrückt: Pin ist LOW (verbunden mit GND)

#### 6. Akku-Ladung & Backup

```
18650 Holder (+) ─────── TP4056 B+
18650 Holder (-) ────── TP4056 B-

TP4056 IN+ ───────────── Netzteil 5V (z.B. USB-Netzteil)
TP4056 IN- ───────────── GND

TP4056 OUT+ ──────────── MT3608 IN+
TP4056 OUT- ──────────── MT3608 IN-
```

**Wichtig:**
- TP4056 MIT Schutzschaltung verwenden (verhindert Tiefentladung!)
- MT3608 auf exakt 5,0V einstellen (mit Multimeter messen!)
- 18650er Akkus sollten gematcht sein (gleiche Kapazität/Alter)

---

#### Kompletter Schaltplan (vereinfacht)

```
                    +5V (von MT3608)
                         │
        ┌────────────────┼────────────────┐
        │                │                │
       [+]              [+]              [+]
        │                │                │
    ┌───┴───┐        ┌───┴───┐        ┌───┴───┐
    │ D1    │        │DS3231 │        │ LM393 │
    │ Mini  │        │  RTC  │        │       │
    └──────┘        └──────        └───┬───┘
        │                │                │
        │ D5 ──┬──10kΩ──── GND          │ Pin 7
        │      │                          │
        │      └── MOSFET Gate            ──220Ω── LED(+)
        │                                   │
        │      Motor                        │
        │      (+) ── +5V                  LED(-)
        │      (-) ─ MOSFET Drain         │
        │         │                        GND
        │        1N4007
        │         │
        │         └──────── GND
        │
        │ D7 ──┬── Taster ── GND
        │      │
        │      └──10kΩ── +5V
        │
        │ D6 ────────────── LM393 Pin 7
        │
        │ D0 ────────────── DS3231 SQW
        │
       GND ──────────────── GND (alles verbinden!)
```

---

### Aufbau-Tipps

#### 1. Test-Reihenfolge:
1. Erst Stromversorgung aufbauen und testen (MT3608 auf 5V einstellen)
2. D1 Mini + RTC testen (I2C Scanner Sketch)
3. Motorsteuerung separat testen
4. Klemmschutz-Schaltung testen (mit Multimeter)
5. Alles zusammenbauen

#### 2. Sicherheit:
- **Freilaufdiode ist PFLICHT!** Sonst geht der MOSFET/ESP kaputt
- **10kΩ Pull-Down am MOSFET Gate** verhindert zufälliges Einschalten
- **TP4056 mit Schutzschaltung** verhindert Akku-Tiefentladung
- **Shunt-Widerstand ausreichend dimensionieren** (mindestens 2W, besser 5W)

#### 3. Gehäuse:
- Elektronik und Futter räumlich trennen!
- Belüftung für Akkus vorsehen
- Manuellen Taster von außen zugänglich machen
- LED für Klemmschutz gut sichtbar platzieren

#### 4. Klemmschutz kalibrieren:
1. Motor ohne Last laufen lassen, Strom messen (z.B. 300mA)
2. Motor blockieren, Strom messen (z.B. 1000mA)
3. Schwellwert dazwischen einstellen (z.B. 800mA = 0,08V am Shunt)
4. Potentiometer so einstellen, dass LED bei Blockierung leuchtet

---

### Einkaufsliste

| Teil | Menge | Preis ca. | Wo |
|------|-------|-----------|-----|
| Wemos D1 Mini | 1 | 3€ | Amazon/AliExpress |
| DS3231 RTC | 1 | 4€ | Amazon/AliExpress |
| IRLZ44N MOSFET | 1 | 1€ | Reichelt/Conrad |
| 1N4007 Diode | 1 | 0,10€ | Reichelt/Conrad |
| LM393 | 1 | 0,50€ | Reichelt/Conrad |
| 0,1Ω 5W Widerstand | 1 | 1€ | Reichelt/Conrad |
| 18650 Akku | 2 | 10€ | Amazon (Original!) |
| 18650 Dual Halter | 1 | 3€ | Amazon/AliExpress |
| TP4056 mit Schutz | 1 | 2€ | Amazon/AliExpress |
| MT3608 Step-Up | 1 | 2€ | Amazon/AliExpress |
| Taster 12mm | 1 | 1€ | Reichelt/Conrad |
| Widerstände/Kondensatoren | - | 5€ | Reichelt/Conrad |
| **Gesamt** | | **~35€** | |

---

## English Version

### Parts List

#### Core Components
- **1x Wemos D1 Mini** (ESP8266)
- **1x DS3231 RTC Module** (NOT DS1307!)
- **1x IRLZ44N or IRLB8721 MOSFET** (Logic-Level N-Channel)
- **1x 1N4007 Freewheeling Diode**
- **1x 10kΩ Resistor** (MOSFET Gate Pull-Down)

#### Jam Protection
- **1x LM393 Comparator**
- **1x 0.1Ω Shunt Resistor** (2-5W)
- **1x Red LED** (5mm)
- **1x 220Ω Resistor** (LED)
- **1x 10kΩ Potentiometer** (threshold adjustment)
- **1x 1kΩ Resistor**

#### Power Supply
- **2x 18650 Li-Ion Battery** (~3000mAh each, e.g. Panasonic NCR18650B)
- **1x 18650 Dual Holder** (parallel)
- **1x TP4056 Charging Module** (with DW01+8205A protection!)
- **1x MT3608 Step-Up Converter** (set to 5V)
- **1x 1000µF Capacitor** (10V, buffer)

#### Controls
- **1x Push Button** (NO, 12mm+)
- **1x 10kΩ Resistor** (Pull-Up)
- **Misc:** Jumper wires, heat shrink, breadboard/PCB

**Total cost:** ~35€

---

### Wiring Overview

#### Power Bus (5V)
```
MT3608 OUT (5V) ──┬── D1 Mini VCC
                  ├── DS3231 VCC
                  ├── LM393 VCC (Pin 8)
                  └── Button (via Pull-Up)

MT3608 GND ───────┬── D1 Mini GND
                  ├── DS3231 GND
                  ├── LM393 GND (Pin 4)
                  ├── MOSFET Source
                  └── Shunt Resistor
```

#### RTC (DS3231)
| DS3231 | D1 Mini |
|--------|---------|
| VCC    | 5V      |
| GND    | GND     |
| SDA    | D2 (GPIO4) |
| SCL    | D1 (GPIO5) |
| SQW    | D0 (GPIO16) — DeepSleep wake-up |

#### Motor Control
```
D1 Mini D5 (GPIO14) ─┬── 10kΩ ── GND (Pull-Down)
                      └── MOSFET Gate

Motor (+) ──────────── 5V
Motor (-) ──────────── MOSFET Drain
1N4007 ─────────────── Across motor (cathode to +5V)
MOSFET Source ──────── GND
```

#### Jam Protection (Hardware LED)
```
Motor (+) ── [0.1Ω Shunt] ──┬── GND
                             │
LM393 (+) Pin 3 ─────────────┘
LM393 (-) Pin 2 ──────────── Potentiometer wiper
LM393 Output Pin 7 ─┬── 220Ω ── LED(+) ── GND
                    └── D1 Mini D6 (GPIO12) — software logging
```

**How it works:** When motor current exceeds threshold (e.g. 800mA = 80mV on shunt), LM393 triggers → LED lights up independently of ESP.

#### Manual Button
```
D1 Mini D7 (GPIO13) ──┬── Button ── GND
                      └── 10kΩ ── 5V (Pull-Up)
```

#### Battery & Charging
```
18650 Holder ─────── TP4056 B+/B-
Mains 5V ──────────── TP4056 IN+/IN-
TP4056 OUT ────────── MT3608 IN
```

---

### Build Tips

#### 1. Test Sequence:
1. ✅ Build and test power supply (MT3608 → 5V)
2. ✅ Test D1 Mini + RTC (I2C scanner sketch)
3. ✅ Test motor control separately
4. ✅ Test jam protection circuit (with multimeter)
5. ✅ Integrate everything

#### 2. Safety Notes:
- **Freewheeling diode is MANDATORY** — protects MOSFET/ESP from voltage spikes
- **10kΩ Pull-Down on MOSFET Gate** — prevents accidental motor activation during ESP crash/reboot
- **TP4056 MUST have protection circuit** — prevents battery deep discharge
- **Calibrate MT3608 to exactly 5.0V** before connecting anything
- **Match 18650 cells** — same capacity, age, and brand
- **Keep electronics separate from food** — avoid contamination and moisture

#### 3. Enclosure:
- Keep electronics and food spatially separated!
- Provide ventilation for batteries
- Make manual button accessible from outside
- Place jam protection LED in visible location

#### 4. Calibration Steps:
1. Run motor without load, measure current (e.g. 300mA)
2. Block motor, measure current (e.g. 1000mA)
3. Set threshold between (e.g. 800mA = 80mV on 0.1Ω shunt)
4. Adjust potentiometer until LED lights up only when motor is blocked

---

### Shopping List

| Part | Qty | Price approx. | Where |
|------|-----|---------------|-------|
| Wemos D1 Mini | 1 | 3€ | Amazon/AliExpress |
| DS3231 RTC | 1 | 4€ | Amazon/AliExpress |
| IRLZ44N MOSFET | 1 | 1€ | Reichelt/Conrad |
| 1N4007 Diode | 1 | 0.10€ | Reichelt/Conrad |
| LM393 | 1 | 0.50€ | Reichelt/Conrad |
| 0.1Ω 5W Resistor | 1 | 1€ | Reichelt/Conrad |
| 18650 Battery | 2 | 10€ | Amazon (Original!) |
| 18650 Dual Holder | 1 | 3€ | Amazon/AliExpress |
| TP4056 with protection | 1 | 2€ | Amazon/AliExpress |
| MT3608 Step-Up | 1 | 2€ | Amazon/AliExpress |
| 12mm Push Button | 1 | 1€ | Reichelt/Conrad |
| Resistors/Capacitors | - | 5€ | Reichelt/Conrad |
| **Total** | | **~35€** | |

---
### Important Notes:

1. **Upload HTML:** You must save the HTML from Parts 1 & 2 as `index.html`, compress it with gzip, and upload it to the ESP using the **ESP8266 LittleFS Uploader** plugin in the Arduino IDE.
2. **Install Libraries:**

   * `RTClib` by Adafruit
   * `PubSubClient` by Nick O'Leary
   * `ArduinoJson` by Benoit Blanchon (v6.x)
   * `NTPClient` by Fabrice Weinberg
3. **Board Settings:**

   * Board: LOLIN(WEMOS) D1 R2 & mini
   * Flash Size: 4MB (FS:2MB OTA:~1019KB)
   * Upload Speed: 921600
   *

---
## Next Steps

After completing the hardware:

1. **Flash Firmware** - Upload the Arduino/PlatformIO code to the D1 Mini
2. **Configure WiFi** - Connect to the access point and configure your network
3. **Set Feeding Schedule** - Use the web interface to program feeding times
4. **Test Everything** - Verify all functions work correctly
5. **Calibrate Jam Protection** - Fine-tune the current threshold
6. **Install in Enclosure** - Mount everything safely


---

*Document Version: 1.0 | Last Updated: 2026*
