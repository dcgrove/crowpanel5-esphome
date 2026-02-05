#include "esp32_p4_rgb_display.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP_IDF

namespace esphome {
namespace esp32_p4_rgb_display {

static const char *const TAG = "esp32_p4_rgb_display";

void ESP32P4RGBDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ESP32-P4 RGB Display...");

  // Validate pin configuration
  if (this->data_pins_.size() != 16) {
    ESP_LOGE(TAG, "Exactly 16 data pins are required for RGB565 mode");
    this->mark_failed();
    return;
  }

  if (this->de_pin_ < 0 || this->pclk_pin_ < 0 || this->hsync_pin_ < 0 || this->vsync_pin_ < 0) {
    ESP_LOGE(TAG, "DE, PCLK, HSYNC, and VSYNC pins must be configured");
    this->mark_failed();
    return;
  }

  // Configure RGB panel (matching factory configuration)
  esp_lcd_rgb_panel_config_t panel_config = {};
  panel_config.data_width = 16;  // RGB565 mode
  panel_config.dma_burst_size = 64;
  panel_config.num_fbs = 2;  // Double buffering for anti-tearing
  panel_config.clk_src = LCD_CLK_SRC_DEFAULT;

  // Use bounce buffer to reduce tearing (factory uses 20 lines)
  panel_config.bounce_buffer_size_px = 20 * this->width_;

  panel_config.disp_gpio_num = -1;  // No display enable pin
  panel_config.pclk_gpio_num = this->pclk_pin_;
  panel_config.vsync_gpio_num = this->vsync_pin_;
  panel_config.hsync_gpio_num = this->hsync_pin_;
  panel_config.de_gpio_num = this->de_pin_;

  // Set data pins
  for (size_t i = 0; i < 16; i++) {
    panel_config.data_gpio_nums[i] = this->data_pins_[i];
  }

  // Set timing parameters
  panel_config.timings.pclk_hz = this->pclk_frequency_;
  panel_config.timings.h_res = this->width_;
  panel_config.timings.v_res = this->height_;
  panel_config.timings.hsync_back_porch = this->hsync_back_porch_;
  panel_config.timings.hsync_front_porch = this->hsync_front_porch_;
  panel_config.timings.hsync_pulse_width = this->hsync_pulse_width_;
  panel_config.timings.vsync_back_porch = this->vsync_back_porch_;
  panel_config.timings.vsync_front_porch = this->vsync_front_porch_;
  panel_config.timings.vsync_pulse_width = this->vsync_pulse_width_;

  // Set timing flags
  panel_config.timings.flags.hsync_idle_low = false;
  panel_config.timings.flags.vsync_idle_low = false;
  panel_config.timings.flags.de_idle_high = false;
  panel_config.timings.flags.pclk_active_neg = this->pclk_inverted_;
  panel_config.timings.flags.pclk_idle_high = !this->pclk_inverted_;

  // Allocate frame buffer in PSRAM
  panel_config.flags.fb_in_psram = true;

  // Create RGB panel
  esp_err_t err = esp_lcd_new_rgb_panel(&panel_config, &this->panel_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create RGB panel: %s", esp_err_to_name(err));
    this->mark_failed();
    return;
  }

  // Reset and initialize panel
  err = esp_lcd_panel_reset(this->panel_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to reset panel: %s", esp_err_to_name(err));
    this->mark_failed();
    return;
  }

  err = esp_lcd_panel_init(this->panel_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize panel: %s", esp_err_to_name(err));
    this->mark_failed();
    return;
  }

  // Get frame buffer pointer
  esp_lcd_rgb_panel_get_frame_buffer(this->panel_handle_, 1, &this->frame_buffer_);

  if (this->frame_buffer_ == nullptr) {
    ESP_LOGE(TAG, "Failed to get frame buffer");
    this->mark_failed();
    return;
  }

  // Clear the framebuffer
  size_t buffer_size = this->width_ * this->height_ * sizeof(uint16_t);
  memset(this->frame_buffer_, 0, buffer_size);

  ESP_LOGCONFIG(TAG, "ESP32-P4 RGB Display initialized");
  ESP_LOGCONFIG(TAG, "  Frame buffer: %p", this->frame_buffer_);
  ESP_LOGCONFIG(TAG, "  Double buffering: enabled (num_fbs=2)");
  ESP_LOGCONFIG(TAG, "  Bounce buffer: %d pixels (20 lines)", 20 * this->width_);
}

void ESP32P4RGBDisplay::update() {
  this->do_update_();
}

void ESP32P4RGBDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "ESP32-P4 RGB Display:");
  ESP_LOGCONFIG(TAG, "  Width: %d", this->width_);
  ESP_LOGCONFIG(TAG, "  Height: %d", this->height_);
  ESP_LOGCONFIG(TAG, "  PCLK Frequency: %u Hz", this->pclk_frequency_);
  ESP_LOGCONFIG(TAG, "  PCLK Inverted: %s", YESNO(this->pclk_inverted_));
  ESP_LOGCONFIG(TAG, "  DE Pin: GPIO%d", this->de_pin_);
  ESP_LOGCONFIG(TAG, "  PCLK Pin: GPIO%d", this->pclk_pin_);
  ESP_LOGCONFIG(TAG, "  HSYNC Pin: GPIO%d", this->hsync_pin_);
  ESP_LOGCONFIG(TAG, "  VSYNC Pin: GPIO%d", this->vsync_pin_);
  ESP_LOGCONFIG(TAG, "  HSYNC: pulse=%d, bp=%d, fp=%d",
                this->hsync_pulse_width_, this->hsync_back_porch_, this->hsync_front_porch_);
  ESP_LOGCONFIG(TAG, "  VSYNC: pulse=%d, bp=%d, fp=%d",
                this->vsync_pulse_width_, this->vsync_back_porch_, this->vsync_front_porch_);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Setup failed!");
  }
}

void ESP32P4RGBDisplay::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || x >= this->width_ || y < 0 || y >= this->height_) {
    return;
  }

  if (this->frame_buffer_ == nullptr) {
    return;
  }

  // Convert RGB888 to RGB565
  uint16_t color565 = ((color.red & 0xF8) << 8) |
                      ((color.green & 0xFC) << 3) |
                      (color.blue >> 3);

  // Write to frame buffer
  uint16_t *fb = (uint16_t *)this->frame_buffer_;
  fb[y * this->width_ + x] = color565;
}

}  // namespace esp32_p4_rgb_display
}  // namespace esphome

#endif  // USE_ESP_IDF
