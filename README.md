## Deej Firmware

<img src="img/logo.svg" alt="drawing" width="150"/><br/>

Firmware repo for Deej, a StreamDeck like DIY mixer. 

### Companion App

The companion app allows you to connect the Deej device to your computer. 

[GitHub Repo (Deej-Companion)](https://github.com/UnoMartino/Deej-Companion) 

### Flashing

Flash the firmware file to your device at offset 0x0 using your favorite tool (or use this browser-based utility [Adafruit_WebSerial_ESPTool](https://adafruit.github.io/Adafruit_WebSerial_ESPTool/)).


### Adapting/building

If you want to make changes or build your own firmware you can use PlatformIO. 

After compiling the firmware in PIO you can use the provided merger script. It will create a combined bin file in the `.pio` folder.

From the root of the project run:
```
.\merge_bin.bat
```

### Notes

You are free to modify the project according to the license.
