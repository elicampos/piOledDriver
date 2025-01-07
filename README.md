# piOledDriver

**GitHub Repo:** [https://github.com/elicampos/piOledDriver](https://github.com/elicampos/piOledDriver)  
**Branch:** master  

A simple C++ project demonstrating how to drive an SH1106/SSD1306-like OLED display via SPI and gather CPU usage stats (using `mpstat`). Designed for a Raspberry Pi 5 or similar Linux SBC with `libgpiod` for GPIO access.

---

## Table of Contents
- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Software Dependencies](#software-dependencies)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Notes & Caveats](#notes--caveats)
- [License](#license)

---

## Overview

1. **SPI Communication**  
   - Leverages `/dev/spidev0.0` in write-only mode to send commands/data to the OLED.
   - Uses `ioctl()` calls to configure SPI mode, speed, and bits per word.

2. **OLED Display (SH1106)**  
   - Contains a 5×7 font array and functions to draw text, clear the screen, and perform initialization steps (contrast, orientation, charge pump, etc.).
   - The display code offsets columns by 2 pixels (standard for many SH1106 modules).

3. **CPU Stats**  
   - Runs `mpstat -P ALL 1 1` to output CPU usage into a text file, then parses the “Average:     all” line to extract `user`, `system`, `irq`, and `idle` usage.

4. **GPIO Control via libgpiod**  
   - Uses `gpiod_chip_open("/dev/gpiochip0")` to request two output lines:
     - **DC (Data/Command)**: Typically `GPIO9` on Raspberry Pi 5.
     - **RESET**: Typically `GPIO25` on Raspberry Pi 5.

---

## Hardware Requirements

1. **Raspberry Pi 5** (or similar board) with:
   - SPI enabled via `raspi-config`.
   - Accessible `/dev/spidev0.0` and `/dev/gpiochip0`.
2. **SH1106-based 128×64 OLED** wired to:
   - **SPI**: `CE0`, `SCLK` (GPIO11), `MOSI` (GPIO10)
   - **DC**: GPIO9 (or your chosen pin)
   - **RESET**: GPIO25 (or your chosen pin)
3. A stable **3.3V** (or 5V) supply, depending on your OLED module’s requirements.

---

## Software Dependencies

- **C++17** (or later) compiler (e.g., `g++`).
- **libgpiod** (for GPIO line management).
- **mpstat** (typically in the `sysstat` package) for CPU usage measurement.
- **SPI** drivers enabled on the Pi (`spidev` kernel module).

Install on Debian-based systems (including Raspberry Pi OS):

```bash
sudo apt-get update
sudo apt-get install -y gpiod libgpiod-dev sysstat
