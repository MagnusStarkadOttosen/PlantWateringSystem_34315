#include "DisplayController.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Arduino.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Screen state (keeps memory between calls)
static int currentScreen = 0;
static unsigned long lastSwitchTime = 0;
const unsigned long screenInterval = 4000; // 4 seconds

void initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        // If display fails, just stop everything
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
}

void updateDisplay(const SystemState& state) {

    display.clearDisplay();
    display.setTextSize(1);

    // ===== TOP LEFT =====
    display.setCursor(0, 0);
    display.print("FAN");
    display.setCursor(0, 10);
    display.print(state.fanActive ? "ON" : "OFF");

    // ===== TOP RIGHT =====
    display.setCursor(64, 0);
    display.print("SOIL");
    display.setCursor(64, 10);
    display.print(state.soilDry ? "DRY" : "WET");

    // ===== BOTTOM LEFT =====
    display.setCursor(0, 32);
    display.print("PUMP");
    display.setCursor(0, 42);
    display.print(state.pumpActive ? "ON" : "OFF");

    // ===== BOTTOM RIGHT =====
    display.setCursor(64, 32);
    display.print("CYCLE");
    display.setCursor(64, 42);
    display.print(state.fanCycleEnabled ? "YES" : "NO");

    display.display();
}