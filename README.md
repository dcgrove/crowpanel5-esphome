# CrowPanel 5.0" ESP32-P4 ESPHome Configuration

ESPHome configuration for the [Elecrow CrowPanel 5.0" ESP32-P4](https://www.elecrow.com/crowpanel-esp32-p4-5-0-display.html) (800x480 RGB parallel display). Includes a custom display driver for the ESP32-P4's built-in LCD peripheral and an LVGL-based coffee station UI.

## Hardware

- **Board:** Elecrow CrowPanel ESP32-P4 5.0"
- **MCU:** ESP32-P4 with 32MB PSRAM (HEX mode)
- **Display:** 5.0" 800x480 RGB565 parallel LCD
- **Touch:** GT911 capacitive touchscreen (I2C)
- **WiFi:** ESP32-C6 co-processor via ESP-Hosted (SDIO)
- **Audio:** Dual NS4168 Class D amplifiers (I2S)
- **I2C Peripherals:** STC8 MCU (backlight/GPIO control at 0x2F), NAU7802 load cell amplifier
- **Flash:** 16MB QIO

## Features

- Custom `esp32_p4_rgb_display` component for the ESP32-P4 RGB LCD peripheral
- LVGL touchscreen UI with dark theme
- Brew method selector (Chemex, Kalita Wave, AeroPress, Clever Dripper, French Press, Moka Pot, Cold Brew)
- NAU7802 load cell integration for coffee scale with tare and auto-stop
- Grinder control with auto-stop at target weight
- Brew timer integration via Home Assistant
- Bookoo BLE scale integration (brewer weight + water target calculation)
- Smart kettle temperature display
- Weather and outside temperature in header bar
- Backlight brightness control with scheduled on/off
- I2S audio with RTTTL tones for button feedback
- Per-sound enable/disable switches

## Repository Structure

```
crowpanel5-esphome/
├── crowpanel5.yaml                          # Main ESPHome configuration
├── secrets.yaml.example                     # Template for secrets
├── components/
│   └── esp32_p4_rgb_display/                # Custom display component
│       ├── __init__.py
│       ├── display.py                       # ESPHome config schema
│       ├── esp32_p4_rgb_display.h           # C++ header
│       └── esp32_p4_rgb_display.cpp         # C++ implementation
└── fonts/
    └── materialdesignicons-webfont.ttf      # MDI icon font
```

## Setup Instructions

### 1. Prerequisites

- [Home Assistant](https://www.home-assistant.io/) with the [ESPHome add-on](https://esphome.io/guides/getting_started_hassio.html) installed
- ESPHome 2024.12+ (ESP32-P4 and `esp32_hosted` support required)
- Elecrow CrowPanel 5.0" ESP32-P4 hardware

### 2. Clone This Repository

Clone into your ESPHome configuration directory:

```bash
cd /config/esphome
git clone https://github.com/dcgrove/crowpanel5-esphome.git
```

Or copy the files manually into your ESPHome config folder so the structure looks like:

```
/config/esphome/
├── crowpanel5.yaml
├── secrets.yaml          # Your secrets (not included)
├── components/
│   └── esp32_p4_rgb_display/
│       └── ...
└── fonts/
    └── materialdesignicons-webfont.ttf
```

### 3. Create secrets.yaml

Copy the example and fill in your values:

```bash
cp secrets.yaml.example secrets.yaml
```

Edit `secrets.yaml` with your credentials:

```yaml
wifi_ssid: "YourWiFiNetwork"
wifi_password: "YourWiFiPassword"
api_pw: "generate-a-random-api-key"
ota_pw: "your-ota-password"
fallback_pw: "fallback-hotspot-password"
```

To generate an API encryption key:

```bash
python3 -c "import secrets, base64; print(base64.b64encode(secrets.token_bytes(32)).decode())"
```

### 4. Customize Entity IDs

The YAML references several Home Assistant entities that you'll need to update for your setup. Search for `NOTE:` comments in `crowpanel5.yaml`:

| Entity ID | Purpose | What to Change |
|---|---|---|
| `switch.grinder` | Smart plug controlling your grinder | Your grinder switch entity |
| `sensor.bookoo_sc_852704_battery` | Bookoo BLE scale battery | Your Bookoo scale entity (or remove) |
| `sensor.bookoo_sc_852704_weight` | Bookoo BLE scale weight | Your Bookoo scale entity (or remove) |
| `sensor.cosori_kettle_temperature` | Smart kettle temperature | Your kettle sensor (or remove) |
| `sensor.openweathermap_feels_like_temperature` | Outside temperature | Your weather sensor |
| `weather.forecast_home` | Weather condition | Your weather entity |
| `input_number.smart_scale_initial_zero` | Scale zero calibration | Create this helper in HA |
| `timer.coffee_timer` | Brew timer | Create this timer in HA |
| `script.start_coffee_timer` | Timer start script | Create this script in HA |

### 5. Home Assistant Helpers

Create these helpers in Home Assistant (Settings > Devices & Services > Helpers):

**input_number.smart_scale_initial_zero:**
- Type: Number
- Min: -1000000
- Max: 1000000
- Step: 1
- Used to persist the NAU7802 zero calibration point

**timer.coffee_timer:**
- Type: Timer
- Used for brew method countdown timers

**script.start_coffee_timer:**

```yaml
# In configuration.yaml or scripts.yaml
script:
  start_coffee_timer:
    alias: Start Coffee Timer
    fields:
      duration_formatted:
        description: "Duration in HH:MM:SS format"
    sequence:
      - service: timer.start
        target:
          entity_id: timer.coffee_timer
        data:
          duration: "{{ duration_formatted }}"
```

### 6. Calibrate the Scale

The NAU7802 load cell requires calibration for your specific hardware:

1. **Find your zero point:** With nothing on the scale, note the raw NAU7802 value shown on the info card. Update `initial_zero` in the globals section.

2. **Calculate counts per gram:** Place a known weight on the scale and note the raw value. Calculate:
   ```
   counts_per_gram = (raw_with_weight - raw_empty) / known_weight_in_grams
   ```
   Update the `counts_per_gram` value in the coffee_scale filter lambda.

3. **Adjust cup offset:** If you use a container on the scale, weigh it and update the offset value (default `2.8`) in the calibration lambda.

### 7. Compile and Flash

From the ESPHome dashboard:

1. Click the three dots on the CrowPanel 5 device
2. Click "Install"
3. Choose "Plug into this computer" for first-time flashing via USB
4. Subsequent updates can use OTA

## Pin Reference

### Display (RGB565 Parallel)
| Signal | GPIO | Notes |
|--------|------|-------|
| PCLK | 3 | 25MHz pixel clock |
| DE | 2 | Data enable |
| HSYNC | 40 | Horizontal sync |
| VSYNC | 41 | Vertical sync |
| DATA0-4 (R) | 8,7,6,5,4 | Red channel |
| DATA5-10 (G) | 14,13,12,11,10,9 | Green channel |
| DATA11-15 (B) | 19,18,17,16,15 | Blue channel |

### I2C Bus
| Signal | GPIO |
|--------|------|
| SDA | 45 |
| SCL | 46 |

### Touchscreen (GT911)
| Signal | GPIO |
|--------|------|
| INT | 42 |
| RST | 36 |

### I2S Audio
| Signal | GPIO |
|--------|------|
| LRCLK | 21 |
| BCLK | 22 |
| DOUT | 23 |

### ESP-Hosted WiFi (ESP32-C6 SDIO)
| Signal | GPIO |
|--------|------|
| CMD | 54 |
| CLK | 53 |
| D0-D3 | 52,51,50,49 |
| RESET | 20 |

### SD Card (not used)
| Signal | GPIO |
|--------|------|
| CLK | 43 |
| CMD | 44 |
| D0 | 39 |

### Other
| Signal | GPIO |
|--------|------|
| Boot Button | 0 |

## Custom Display Component

The `esp32_p4_rgb_display` component is a custom ESPHome external component that drives the ESP32-P4's built-in RGB LCD peripheral. It uses the ESP-IDF `esp_lcd_panel_rgb` API with:

- 16-bit RGB565 color mode
- Double-buffered frame buffers in PSRAM
- 20-line bounce buffer for reduced tearing
- 25MHz pixel clock (~42Hz refresh rate)

This component is necessary because ESPHome does not yet have built-in support for the ESP32-P4's RGB LCD peripheral.

## Adapting for Your Own Project

This configuration is built as a coffee station controller, but the display driver and hardware setup work for any project. To use the CrowPanel 5" with your own UI:

1. Keep the `external_components`, `esp32`, `psram`, `esp32_hosted`, `i2c`, `touchscreen`, and `display` sections as-is -- these are hardware-specific.
2. Keep the `on_boot` backlight initialization lambdas.
3. Replace the `lvgl` pages/widgets section with your own UI.
4. Remove the coffee-specific sensors, globals, and buttons you don't need.

## License

MIT
