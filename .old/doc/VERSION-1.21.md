# FeedMate v1.21

v1.21 korrigiert die native USB-Konfiguration aus v1.20. C3, C6, S2 und S3 erhalten nun gemeinsam `ARDUINO_USB_MODE=1` und `ARDUINO_USB_CDC_ON_BOOT=1`, sodass der Arduino-Core das vorhandene `HWCDCSerial` verwendet. AP-Prüfung, automatische Wiederholungsversuche und die festen Erstzugänge `feedmate` beziehungsweise `FeedMateAdmin` bleiben enthalten; die Demo verwendet weiterhin `admin`.
