# AMPi-Display
AMPi Arduino Nano code to control ST7735 TFT Screen and Rotary Encoder interface

AMPi is a project that integrates a mains electricity power supply with an audio amplifier, an 4-channel audio relay switcher, input audio  transformers, Raspberry PI with DAC, ST7735 based TFT display, Arduino Nano (to control the Raspberry PI over serial and the built-in screen) to make a completely integrated digital audio solution that supports the latest high quality audio streaming and smart speaker Bluetooth 5.0 functionality in one box.

This repository is the code for the Arduino Nano.

First prototype (WIP) with Raspberry Pi Model 3 and a Arduino Nano

![Front Side (first prototype)](https://raw.githubusercontent.com/bjaan/AMPi-Display/main/firstprototype-top.jpg)
![Back Side (first prototype)](https://raw.githubusercontent.com/bjaan/AMPi-Display/main/firstprototype-back.jpg)

Arduino Nano Pins - use a 3.3V/5V level shifter! as this Arduino Nano runs on 5V, while the display & Raspberry PI requires 3.3V

    Display (Pin) - Arduino Nano
    ----------------------------
    GND  (1) - GND
    VCC  (2) - 3.3V
    SCK  (3) - D13
    SDA  (4) - D11
    RES  (5) - RST
    RS   (6) - D9 (DC)
    CS   (7) - D10
    LEDA (8) - 3.3V

   
    Other - Arduino Nano
    ----------------------------
    Raspberry Pi Relay Signal - D8
    Rotary Encoder Clock - D2
    Rotary Encoder Data - D3
    Rotary Encoder Switch - D4
	

Parts used:
* Nobsound HiFi Bluetooth 5.0 Amplifier Board 2.0 Channel Stereo Audio Amp DIY Module Digital Class D Amp APTX-HD https://nl.aliexpress.com/item/1005001412408638.html
* AZ-Delivery 1.77 inch SPI TFT display and 128x160 pixels for Arduino https://www.az-delivery.de/en/products/1-77-zoll-spi-tft-display
* Audio Relay Switcher https://www.aliexpress.com/item/4000715642555.html
* Raspberry Pi Model 3
* Nickel Alloy Audio Transformers 600:600 Ohm - I needed two; one for each audio channel https://www.aliexpress.com/item/32827431499.html
* 24 LED Stereo VU Meter https://nl.aliexpress.com/item/4000136566150.html
* 3.3V/5V level-shifter to protect the Raspberry Pi & Display - I needed two https://www.amazon.de/-/en/gp/product/B07LG6RK7L
* AZ-Delivery Arduino Nano https://www.az-delivery.de/en/products/nano-v3-mit-ftdi-arduino-kompatibel
* Thick Micro USB Cable to power the Raspberry PI 3 https://www.aliexpress.com/item/33027891253.html
* Micro SD Card extension cable https://www.aliexpress.com/item/4000405842609.html
* 24V USB Car Charger - to power the Raspberry PI, I used the one that came with a PSVITA
* 3mm thick aluminum plates cut to size
* 100-40V Switching Power Supply 24V https://www.amazon.de/gp/product/B01H5MK3OI/
* Speaker Terminals https://www.amazon.de/gp/product/B000L0ZMWK
* Electric (braided) wire to connect speaker terminals and mains electricity
* Prototype boards to attach components (Arduino Nano & Audio Transformers & connectors to the display and Raspberry Pi)
* Wires https://www.aliexpress.com/item/32822880152.html
* Knobs https://www.amazon.de/TOOGOO-Knurled-Insert-Potentiometer-Control/dp/B00QLHKZB0
* Kuman PIFI Digi DAC+ HiFi DAC Audio Sound Card Module I2S Interface for Raspberry Pi 3 https://www.amazon.de/gp/product/B07SRBBG44
* Fuse holder https://www.amazon.de/fuse-holder-size-5x20-screw/dp/B001C6JSAY
* Fuses to protect 24V circuit https://www.amazon.de/gp/product/B00K1BGUL6
* Mains electricity switch & female connector & fuse holder in one https://www.amazon.de/gp/product/B01H5MK3OI
* Rotary Encoder https://www.amazon.de/gp/product/B07T3672VK
* Jumper Wires and DuPont to connectors to wire everything up
* Spacer screws to attach the Arduino and top plate where the amplifier board rests
* Velleman Case WCAH2507 INSTRUMENT CASE - GREY 250 x 190 x 80mm https://www.velleman.eu/products/view?id=17836
* Speakers - I used repaired 3-way ones from HECO/Summit from the early 80's

Tools used:
* Soldering iron & Solder
* Fabric Adhesive Tape https://www.aliexpress.com/item/4000235805659.html
* Kapton Tape https://www.aliexpress.com/item/4001221342106.html
* Hot glue (sorry, not sorry) to hold the VU-meter and Micro-SD Card Holder *strongly* in place
* Wire Cutters https://www.aliexpress.com/item/32950629549.html

Notes from the TFT supplier in German.

    LEDA kann auch an 5V doch dann wird das Display sehr schnell sehr heiß - was ich nicht für optimal halte.
    Beim Betrieb mit 3.3V ist das Dispklay nur minimal dunkler und bleibt kalt.
    Die tft Funktionen kommen aus der Adafruit Grafik-Bibliothek (Adafruit_GFX) die möglichen Funktionen finden sich unter:
    https://learn.adafruit.com/adafruit-gfx-graphics-library?view=all
    https://cdn-learn.adafruit.com/downloads/pdf/adafruit-gfx-graphics-library.pdf

    ST7735-Chip initialisieren (INITR_BLACKTAB)
    Muss bei AZ-Delivery 1.77'' 160x128px RGB TFT INITR_GREENTAB sein ansonsten Pixelfehler rechts und unten.
    Hinweis: https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/examples/soft_spitftbitmap/soft_spitftbitmap.ino#L52
    Zeile 52-65
