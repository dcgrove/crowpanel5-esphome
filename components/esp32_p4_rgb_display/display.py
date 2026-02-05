import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display
from esphome.const import (
    CONF_ID,
    CONF_DIMENSIONS,
    CONF_HEIGHT,
    CONF_WIDTH,
)

DEPENDENCIES = ["esp32"]

esp32_p4_rgb_display_ns = cg.esphome_ns.namespace("esp32_p4_rgb_display")
ESP32P4RGBDisplay = esp32_p4_rgb_display_ns.class_(
    "ESP32P4RGBDisplay", cg.PollingComponent, display.DisplayBuffer
)

CONF_DATA_PINS = "data_pins"
CONF_DE_PIN = "de_pin"
CONF_PCLK_PIN = "pclk_pin"
CONF_HSYNC_PIN = "hsync_pin"
CONF_VSYNC_PIN = "vsync_pin"
CONF_PCLK_FREQUENCY = "pclk_frequency"
CONF_PCLK_INVERTED = "pclk_inverted"
CONF_HSYNC_PULSE_WIDTH = "hsync_pulse_width"
CONF_HSYNC_BACK_PORCH = "hsync_back_porch"
CONF_HSYNC_FRONT_PORCH = "hsync_front_porch"
CONF_VSYNC_PULSE_WIDTH = "vsync_pulse_width"
CONF_VSYNC_BACK_PORCH = "vsync_back_porch"
CONF_VSYNC_FRONT_PORCH = "vsync_front_porch"

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(ESP32P4RGBDisplay),
            cv.Required(CONF_DIMENSIONS): cv.Schema(
                {
                    cv.Required(CONF_WIDTH): cv.int_range(min=1),
                    cv.Required(CONF_HEIGHT): cv.int_range(min=1),
                }
            ),
            cv.Required(CONF_DATA_PINS): cv.All(
                [pins.internal_gpio_output_pin_number],
                cv.Length(min=16, max=16),
            ),
            cv.Required(CONF_DE_PIN): pins.internal_gpio_output_pin_number,
            cv.Required(CONF_PCLK_PIN): pins.internal_gpio_output_pin_number,
            cv.Required(CONF_HSYNC_PIN): pins.internal_gpio_output_pin_number,
            cv.Required(CONF_VSYNC_PIN): pins.internal_gpio_output_pin_number,
            cv.Optional(CONF_PCLK_FREQUENCY, default="25MHz"): cv.frequency,
            cv.Optional(CONF_PCLK_INVERTED, default=False): cv.boolean,
            cv.Optional(CONF_HSYNC_PULSE_WIDTH, default=4): cv.int_range(min=1),
            cv.Optional(CONF_HSYNC_BACK_PORCH, default=8): cv.int_range(min=1),
            cv.Optional(CONF_HSYNC_FRONT_PORCH, default=8): cv.int_range(min=1),
            cv.Optional(CONF_VSYNC_PULSE_WIDTH, default=4): cv.int_range(min=1),
            cv.Optional(CONF_VSYNC_BACK_PORCH, default=16): cv.int_range(min=1),
            cv.Optional(CONF_VSYNC_FRONT_PORCH, default=16): cv.int_range(min=1),
        }
    ),
    cv.only_with_esp_idf,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    dimensions = config[CONF_DIMENSIONS]
    cg.add(var.set_dimensions(dimensions[CONF_WIDTH], dimensions[CONF_HEIGHT]))

    cg.add(var.set_data_pins(config[CONF_DATA_PINS]))
    cg.add(var.set_de_pin(config[CONF_DE_PIN]))
    cg.add(var.set_pclk_pin(config[CONF_PCLK_PIN]))
    cg.add(var.set_hsync_pin(config[CONF_HSYNC_PIN]))
    cg.add(var.set_vsync_pin(config[CONF_VSYNC_PIN]))

    cg.add(var.set_pclk_frequency(config[CONF_PCLK_FREQUENCY]))
    cg.add(var.set_pclk_inverted(config[CONF_PCLK_INVERTED]))

    cg.add(var.set_hsync_pulse_width(config[CONF_HSYNC_PULSE_WIDTH]))
    cg.add(var.set_hsync_back_porch(config[CONF_HSYNC_BACK_PORCH]))
    cg.add(var.set_hsync_front_porch(config[CONF_HSYNC_FRONT_PORCH]))

    cg.add(var.set_vsync_pulse_width(config[CONF_VSYNC_PULSE_WIDTH]))
    cg.add(var.set_vsync_back_porch(config[CONF_VSYNC_BACK_PORCH]))
    cg.add(var.set_vsync_front_porch(config[CONF_VSYNC_FRONT_PORCH]))
