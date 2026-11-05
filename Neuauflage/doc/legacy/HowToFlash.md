Flashing FeedMate onto the ESP8266 – Step-by-Step
---
## Table of Contents

1. [STEP 1: Prepare the Arduino IDE](#step-1-prepare-the-arduino-ide)  
   1.1 [Install the Arduino IDE](#11-install-the-arduino-ide)  
   1.2 [Add the ESP8266 Board Support](#12-add-the-esp8266-board-support)  
   1.3 [Install Libraries](#13-install-libraries)  
2. [STEP 2: Create the Project Folder](#step-2-create-the-project-folder)  
   2.1 [Alternative: Everything in one file](#alternative-everything-in-one-file)  
3. [STEP 3: Board Settings](#step-3-board-settings)  
4. [STEP 4: Prepare and Upload the HTML File](#step-4-prepare-and-upload-the-html-file)  
   4.1 [Install the LittleFS Uploader Plugin](#41-install-the-littlefs-uploader-plugin)  
   4.2 [Compress the HTML file](#42-compress-the-html-file)  
   4.3 [Upload to LittleFS](#43-upload-to-littlefs)  
5. [STEP 5: Compile and Upload](#step-5-compile-and-upload)  
6. [STEP 6: First Steps After Flashing](#step-6-first-steps-after-flashing)  
   6.1 [Open the Serial Monitor](#61-open-the-serial-monitor)  
   6.2 [Open the Web Interface](#62-open-the-web-interface)  
   6.3 [First Configuration](#63-first-configuration)  
7. [STEP 7: OTA Updates (Optional)](#step-7-ota-updates-optional)  
8. [Troubleshooting](#troubleshooting)  
   8.1 [ESP is not detected](#esp-is-not-detected)  
   8.2 [HTML is not displayed](#html-is-not-displayed)  
   8.3 [Motor does not run](#motor-does-not-run)  
   8.4 [MQTT does not connect](#mqtt-does-not-connect)  
9. [Next Steps](#next-steps)  

# Flashing FeedMate onto the ESP8266 – Step-by-Step

---

## **STEP 1: Prepare the Arduino IDE**

### 1.1 Install the Arduino IDE
Download the latest version from [arduino.cc](https://www.arduino.cc/en/software) and install it.

### 1.2 Add the ESP8266 Board Support
1. Open the Arduino IDE
2. Go to **File → Preferences**
3. Paste this URL into "Additional Board Manager URLs":
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Click OK
5. Go to **Tools → Board → Board Manager**
6. Search for "esp8266" and install **"esp8266 by ESP8266 Community"**

### 1.3 Install Libraries
Go to **Tools → Manage Libraries** and install:

| Library | Author | Version |
|---------|--------|---------|
| **RTClib** | Adafruit | latest |
| **PubSubClient** | Nick O'Leary | latest |
| **ArduinoJson** | Benoit Blanchon | **6.x** (not 7!) |
| **NTPClient** | Fabrice Weinberg | latest |

⚠️ **Important:** ArduinoJson must be version 6.x, not 7.x! The code is written for v6.

---

## **STEP 2: Create the Project Folder**

The Arduino IDE expects a **main file named like the folder**. This is how you set up the project:

1. Create a folder named `FeedMate` (e.g., on the desktop)
2. Inside it, create the following files (using a text editor like VS Code or Notepad++):

```
FeedMate/
├── FeedMate.ino       ← main file (must be named like the folder!)
├── config.h           ← pin definitions & structures
├── storage.cpp        ← LittleFS & configuration
── webserver.cpp      ← web server & API
├── sensors.cpp        ← motor & sensors
├── mqtt_client.cpp    ← MQTT integration
└── schedule.cpp       ← feeding time logic
```

3. Copy the code from the previous answers into the respective files
4. **IMPORTANT:** `FeedMate.ino` must be empty or contain only `#include "config.h"` since the actual code is in `main.cpp`. Rename `main.cpp` to `FeedMate.ino`!

### Alternative: Everything in one file
If the modular structure is too complicated, you can also copy **all files into a single `.ino` file**. Then remove all `#include "config.h"` lines and paste the contents of `config.h` at the very top. For getting started, this is easier.

---

## **STEP 3: Board Settings**

Go to **Tools** and set the following:

| Setting | Value |
|---------|-------|
| Board | **LOLIN(WEMOS) D1 R2 & mini** |
| Upload Using | **Serial** |
| CPU Frequency | **80 MHz** |
| Flash Size | **4MB (FS:2MB OTA:~1019KB)** |
| Debug port | **Disabled** |
| Debug Level | **None** |
| IwIP Variant | **v2 Lower Memory** |
| VTables | **Flash** |
| Exceptions | **Legacy (new can return nullptr)** |
| Erase Flash | **Only Sketch** |
| SSL Support | **All SSL ciphers (most compatible)** |
| Port | **COM3** (or whichever port your ESP uses) |

---

## **STEP 4: Prepare and Upload the HTML File**

This is the tricky part. The HTML must be stored in the ESP’s LittleFS.

### 4.1 Install the LittleFS Uploader Plugin
1. Download the plugin from GitHub: [ESP8266 LittleFS Filesystem Uploader](https://github.com/earlephilhower/arduino-littlefs-upload/releases)
2. Extract the ZIP file
3. Copy the folder `LittleFSUpload` into your Arduino plugins folder:
   - **Windows:** `C:\Users\<YOURNAME>\Documents\Arduino\tools\`
   - **Mac:** `~/Documents/Arduino/tools/`
   - **Linux:** `~/Arduino/tools/`
4. Restart the Arduino IDE

### 4.2 Compress the HTML file
1. Save the HTML from part 1 & 2 as `index.html`
2. Create a subfolder named `data` inside the `FeedMate` folder
3. Copy `index.html` into the `data` folder
4. Compress the file with gzip:
   - **Windows:** Right-click → "Send to" → "Compressed (zipped) folder" (does not work directly for gzip)
   - **Better:** Use [7-Zip](https://www.7-zip.org/) or online tools like [gzip.online](https://gzip.online/)
   - **Linux/Mac:** Terminal: `gzip -k index.html`
5. The compressed file is now named `index.html.gz`

### 4.3 Upload to LittleFS
1. Make sure the `data` folder contains `index.html.gz`
2. Go to **Tools → ESP8266 LittleFS Data Upload**
3. Wait until it says "LittleFS Image Uploaded"

---

## **STEP 5: Compile and Upload**

1. Open `FeedMate.ino` in the Arduino IDE
2. Click the **checkmark** (✓) in the top-left to compile
3. If there are no errors, click the **arrow** (→) to upload
4. Wait until you see "Hard resetting via RTS pin..."

### Common Errors and Solutions:

| Error | Solution |
|--------|----------|
| "espcomm_sync failed" | Wrong port or ESP not in flash mode. Hold the FLASH button and press RST |
| "LittleFS mount failed" | Flash size set incorrectly. Set it to 4MB |
| "ArduinoJson.h: No such file" | Library not installed |
| "Multiple libraries found" | Remove older versions |

---

## **STEP 6: First Steps After Flashing**

### 6.1 Open the Serial Monitor
1. Go to **Tools → Serial Monitor** (or Ctrl+Shift+M)
2. Set **115200 baud**
3. You should see:

   ```
   === FeedMate Booting ===
   LittleFS ready
   Config loaded
   Schedule loaded
   Connecting to WiFi...
   Connected! IP: 192.168.1.XXX
   Web server started
   === FeedMate Ready ===
   ```

### 6.2 Open the Web Interface
1. Open a browser
2. Enter the IP address (e.g., `http://192.168.1.100`)
3. You should see the FeedMate interface

### 6.3 First Configuration
1. Click on "Settings"
2. Go to "WiFi" and enter your WiFi details
3. Save and restart
4. Optional: Enter your MQTT server for Home Assistant

---

## **STEP 7: OTA Updates (Optional)**

After the first flash, you can update **without a cable**:

1. In the browser, go to `http://<IP-OF-ESP>/update`
2. User: `admin`, password: `feedmate`
3. Upload the new `.bin` file

---

## **Troubleshooting**

### ESP is not detected
- Check the USB cable (some are power-only)
- Install drivers: [CH340 driver](http://www.wch-ic.com/downloads/CH341SER_EXE.html)

### HTML is not displayed
- Repeat the LittleFS upload
- Clear browser cache (Ctrl+F5)
- Check the Serial Monitor for "HTML file not found"

### Motor does not run
- Check wiring (see wiring diagram)
- Don’t forget the 10kΩ pull-down on the MOSFET gate!
- Check the Serial Monitor for "Motor started"

### MQTT does not connect
- Check the IP address of the MQTT server
- Check firewall settings
- Check the Serial Monitor for "MQTT failed"

---

## **Next Steps**

Once everything is running:
1. **Design the enclosure** and 3D print it
2. **Test the sensors** (light barrier, limit switch)
3. **Set up Home Assistant integration**
4. **Perform feeding calibration**
