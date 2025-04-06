#include <HardwareSerial.h>
#include <BleKeyboard.h>
#include <driver/ledc.h>

#define BUZZER_PIN 4   // GPIO4 for buzzer
#define LED_PIN 13     // GPIO13 for LED
#define TRIGGER_PIN 5  // GPIO5 for trigger (use GPIO4, GPIO5, or another stable one)

BleKeyboard bleKeyboard("ESP32_Beep", "ESP32", 100);
bool isPlaying = false;  // Track whether trill is playing
bool wasConnected = false;  // Track BLE connection state

void playTrill(bool state) {
    if (state) {
        int frequencies[] = {600, 900};  
        for (int i = 0; i < 10; i++) {   
            ledcWriteTone(0, frequencies[i % 2]);  
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
        }
    } else {
        ledcWriteTone(0, 0);
        digitalWrite(LED_PIN, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    bleKeyboard.begin();

    // Setup PWM for buzzer
    ledcSetup(0, 1000, 8);
    ledcAttachPin(BUZZER_PIN, 0);

    // Setup LED and trigger pin
    pinMode(LED_PIN, OUTPUT);
    pinMode(TRIGGER_PIN, INPUT_PULLDOWN);  // Enable pull-down to make sure it starts low

    Serial.println("Setup complete. Power ON successful.");
}

void loop() {
    // Auto-reconnect BLE if disconnected
    if (!bleKeyboard.isConnected()) {
        if (wasConnected) {
            Serial.println("❌ Bluetooth disconnected. Restarting BLE...");
            bleKeyboard.end();  // Stop Bluetooth
            delay(1000);
            bleKeyboard.begin(); // Restart Bluetooth
            Serial.println("🔄 BLE restarted, waiting for new connection...");
        }
        wasConnected = false;
    } else {
        if (!wasConnected) {
            Serial.println("✅ Bluetooth connected!");
        }
        wasConnected = true;
    }

    // Check if trigger pin reads HIGH (when connected to 3.3V)
    bool triggerState = digitalRead(TRIGGER_PIN) == HIGH;

    // Print the state of the trigger pin for debugging
    Serial.print("Trigger State: ");
    Serial.println(triggerState);

    if (triggerState && !isPlaying) {
        Serial.println("✅ Trigger detected! Playing trill and blinking LED.");
        playTrill(true);
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        isPlaying = true;
    } 
    else if (!triggerState && isPlaying) {
        Serial.println("❌ Trigger released. Stopping trill and LED.");
        playTrill(false);
        isPlaying = false;
    }

    delay(100); // Short delay for stability
}


