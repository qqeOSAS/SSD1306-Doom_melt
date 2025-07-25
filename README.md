# Doom Melt Effect Renderer for SSD1306 using U8g2


   ____ ____  ____  _ _____  ___   __     ____                            __  __      _ _   
  / ___/ ___||  _ \/ |___ / / _ \ / /_   |  _ \  ___   ___  _ __ ___     |  \/  | ___| | |_ 
  \___ \___ \| | | | | |_ \| | | | '_ \  | | | |/ _ \ / _ \| '_ ` _ \    | |\/| |/ _ \ | __|
   ___) |__) | |_| | |___) | |_| | (_) | | |_| | (_) | (_) | | | | | |   | |  | |  __/ | |_ 
  |____/____/|____/|_|____/ \___/ \___/  |____/ \___/ \___/|_| |_| |_|___|_|  |_|\___|_|\__|
                                                                    |_____|



A library for rendering the classic **DOOM "melt" screen transition effect** on OLED displays using the SSD1306 driver and the U8g2 graphics library.

## 🎬 Inspired By

This library was inspired by the iconic DOOM melt screen transition seen in classic games.  
The idea came from this video demonstration:  
🔗 [DOOM transition explained](https://www.youtube.com/watch?v=lUsCXSNhHmI)

After watching it, I decided to implement this effect for microcontrollers with SSD1306 displays.

## ✨ Features

- Smooth DOOM-style "melt" animation from one image to another
- Optimized for SSD1306 128x64 displays
- Built on top of the U8g2 library
- Supports per-column delay to simulate realistic "melting"

## 🔧 Requirements

- [U8g2](https://github.com/olikraus/u8g2) graphics library
- An SSD1306-compatible OLED display (tested on 128x64)
- Arduino-compatible microcontroller

## 📦 Installation

1. Copy the library folder into your Arduino `libraries/` directory
2. Include it in your sketch:

```cpp
#include <SSD1306-Doom_melt.h>
