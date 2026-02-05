#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"

#ifdef USE_ESP_IDF

#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>

namespace esphome {
namespace esp32_p4_rgb_display {

class ESP32P4RGBDisplay : public display::DisplayBuffer {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void set_dimensions(uint16_t width, uint16_t height) {
    this->width_ = width;
    this->height_ = height;
  }

  void set_data_pins(const std::vector<int> &data_pins) { this->data_pins_ = data_pins; }
  void set_de_pin(int pin) { this->de_pin_ = pin; }
  void set_pclk_pin(int pin) { this->pclk_pin_ = pin; }
  void set_hsync_pin(int pin) { this->hsync_pin_ = pin; }
  void set_vsync_pin(int pin) { this->vsync_pin_ = pin; }

  void set_pclk_frequency(uint32_t freq) { this->pclk_frequency_ = freq; }
  void set_pclk_inverted(bool inverted) { this->pclk_inverted_ = inverted; }

  void set_hsync_pulse_width(uint16_t width) { this->hsync_pulse_width_ = width; }
  void set_hsync_back_porch(uint16_t porch) { this->hsync_back_porch_ = porch; }
  void set_hsync_front_porch(uint16_t porch) { this->hsync_front_porch_ = porch; }

  void set_vsync_pulse_width(uint16_t width) { this->vsync_pulse_width_ = width; }
  void set_vsync_back_porch(uint16_t porch) { this->vsync_back_porch_ = porch; }
  void set_vsync_front_porch(uint16_t porch) { this->vsync_front_porch_ = porch; }

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override { return this->width_; }
  int get_height_internal() override { return this->height_; }

  uint16_t width_{800};
  uint16_t height_{480};

  std::vector<int> data_pins_;
  int de_pin_{-1};
  int pclk_pin_{-1};
  int hsync_pin_{-1};
  int vsync_pin_{-1};

  uint32_t pclk_frequency_{25000000};  // 25MHz default
  bool pclk_inverted_{false};

  uint16_t hsync_pulse_width_{4};
  uint16_t hsync_back_porch_{8};
  uint16_t hsync_front_porch_{8};

  uint16_t vsync_pulse_width_{4};
  uint16_t vsync_back_porch_{16};
  uint16_t vsync_front_porch_{16};

  esp_lcd_panel_handle_t panel_handle_{nullptr};
  void *frame_buffer_{nullptr};
};

}  // namespace esp32_p4_rgb_display
}  // namespace esphome

#endif  // USE_ESP_IDF
