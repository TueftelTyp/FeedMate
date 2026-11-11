## 📋 FeedMate - Schematic Summary
![compiled with](https://img.shields.io/badge/compiled%20with-AI-c34148?style=flat&labelColor=555555)
### 🔌 Power Supply (UPS System)
```
5V PSU → TP4056 → 2x 18650 Batteries (parallel) → MT3608 → 5V Rail
```
- **TP4056**: Charges batteries from the PSU, protects against deep discharge.
- **MT3608**: Steps up 3.0-4.2V (Battery) to a stable **5.0V**.
- **IMPORTANT**: Set the MT3608 output to exactly **5.0-5.2V** (measure before connecting the ESP!).

### 🧠 Main Components
1. **Wemos D1 Mini (ESP8266)** - Microcontroller
2. **DS3231 RTC** - Real Time Clock (I²C: D1/D2)
3. **IRLZ44N MOSFET** - Motor switching (D5/GPIO14)
4. **LM393 Comparator** - Hardware jam protection (D6/GPIO12)
5. **0.1Ω Shunt** - Current measurement for jam protection
6. **1N4007 Diode** - Flyback diode at the motor
7. **Optional Sensors**:
   - Light barrier (D7/GPIO13)
   - Hall sensor / Limit switch (D8/GPIO15)
8. **Push Button** - Manual feeding (D3/GPIO0)

---

## 📊 GPIO Pinout FeedMate

| GPIO | Pin | Function | Connection | Pull Resistor |
|------|-----|----------|------------|---------------|
| **GPIO 0** | D3 | Manual Button | SW_MANUAL → GND | Internal Pull-Up |
| **GPIO 4** | D2 | I²C SDA | DS3231 SDA | - |
| **GPIO 5** | D1 | I²C SCL | DS3231 SCL | - |
| **GPIO 12** | D6 | Jam Protection Input | LM393 OUT | - |
| **GPIO 13** | D7 | Light Barrier (optional) | IR_BARRIER OUT | - |
| **GPIO 14** | D5 | Motor Control | MOSFET Gate (via 220Ω) | **10kΩ Pull-Down** ⚠️ |
| **GPIO 15** | D8 | Hall Sensor (optional) | HALL_SENSOR OUT | - |
| **3V3** | - | 3.3V Logic | RTC VCC, Sensors VCC | - |
| **5V** | - | 5V Power | Motor (+), RTC VCC | - |
| **GND** | - | Ground | All GND connections | - |

---

## ⚠️ Critical Warnings

### 1. MOSFET Gate Pull-Down (LIFESAVER!)
```
GPIO14 ──220Ω──┬── Gate (IRLZ44N)
               │
              10kΩ
               │
              GND
```
**Why?** If the ESP crashes or reboots, GPIO14 becomes high-impedance. The 10kΩ resistor guarantees the Gate is pulled to 0V, ensuring the motor **never turns on accidentally**!

### 2. Flyback Diode at Motor
```
Motor (+) ──┬── 5V
            │
           ─┴─ 1N4007 (Cathode/Ring to Motor+)
            │
Motor (-) ─┴── Drain (MOSFET)
```
**Why?** Prevents inductive voltage spikes (up to 100V!) that would destroy the MOSFET and the ESP.

### 3. Remove RTC Battery!
- **Remove the CR2032 from the DS3231 module!**
- Many cheap modules have faulty charging circuits → Explosion hazard.
- The 18650 UPS handles the RTC power supply perfectly.

### 4. Common Ground (Star Point)
Connect all GND connections at **one central point** (Star topology):
- TP4056 OUT-
- MT3608 GND
- ESP GND
- MOSFET Source
- Shunt resistor
- Sensors GND

---

##  FeedMate Bill of Materials (BOM)

| Component | Qty | Important Note |
|-----------|-----|----------------|
| Wemos D1 Mini (ESP8266) | 1x | - |
| DS3231 RTC Module | 1x | **Operate without coin cell battery** |
| TP4056 with Protection | 1x | DW01+8205A chip required |
| MT3608 Step-Up | 1x | Set to 5.0-5.2V |
| IRLZ44N or IRLB8721 MOSFET | 1x | Must be Logic-Level! |
| 1N4007 Diode | 1x | Flyback diode |
| 18650 Li-Ion Batteries | 2x | Connected in parallel |
| LM393 Comparator | 1x | Hardware jam protection |
| 0.1Ω 2W Resistor | 1x | Shunt for current measurement |
| 10kΩ Resistor | 2x | MOSFET Pull-Down + LM393 |
| 220Ω Resistor | 2x | MOSFET Gate + LED |
| Red LED | 1x | Jam protection indicator |
| Push Button | 1x | Manual feeding |
| Fork Light Barrier | 1x | Optional (Food level) |
| A3144 Hall Sensor | 1x | Optional (Rotations) |

