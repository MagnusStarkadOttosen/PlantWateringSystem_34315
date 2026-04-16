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

    unsigned long now = millis();

    // --- screen switching logic ---
    if (now - lastSwitchTime > screenInterval) {
        currentScreen = (currentScreen + 1) % 4;
        lastSwitchTime = now;
    }

    display.clearDisplay();
    display.setCursor(0, 0);

    switch (currentScreen) {

        case 0:
            display.println("== FAN ==");
            display.print("State: ");
            display.println(state.fanActive ? "ON" : "OFF");
            break;

        case 1:
            display.println("== SOIL ==");
            display.print("Moisture: ");
            display.println(state.soilDry ? "DRY" : "WET");
            break;

        case 2:
            display.println("== PUMP ==");
            display.print("State: ");
            display.println(state.pumpActive ? "ON" : "OFF");
            break;

        case 3:
            display.println("== CYCLE ==");
            display.print("Enabled: ");
            display.println(state.fanCycleEnabled ? "YES" : "NO");
            break;
    }

    display.display();
}