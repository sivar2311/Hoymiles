#include <Arduino.h>
#include <Hoymiles.h>
#include <time.h>
#include <WiFi.h>

// Customise the settings to your needs
uint64_t    inv_serial = 0x001122334455UL;
uint64_t    dtu_serial = 0x99978563412UL;
const char* WIFI_SSID  = "your-wifi-ssid";
const char* WIFI_PASS  = "your-wifi-pass";
const char* NTP_SERVER = "pool.ntp.org";
const char* TIME_ZONE  = "CET-1CEST,M3.5.0,M10.5.0/3";

void setupHoymiles() {
    Hoymiles.init();
    Hoymiles.getRadio()->setPALevel(RF24_PA_MAX);
    Hoymiles.getRadio()->setDtuSerial(dtu_serial);
    Hoymiles.setPollInterval(5);
}

void setupInverter() {
    Hoymiles.addInverter("Inverter1", inv_serial);
}

void printStatistics() {
    static unsigned long last_millis;
    unsigned long        current_millis = millis();
    if (current_millis - last_millis < 5000) return;
    last_millis = current_millis;

    uint8_t num_inverters = Hoymiles.getNumInverters();

    for (size_t inv_num = 0; inv_num < num_inverters; inv_num++) {
    
        auto    inv           = Hoymiles.getInverterByPos(inv_num);
        auto    stats         = inv->Statistics();
        uint8_t channel_count = stats->getChannelCount();

        for (uint8_t channel = 0; channel < channel_count + 1; channel++) {
            Serial.printf("Inverter %d Channel %d\r\n", inv_num, channel);
            for (uint8_t field = 0; field < 20; field++) {
                if (stats->hasChannelFieldValue(channel, field)) {
                    float       value = stats->getChannelFieldValue(channel, field);
                    const char* name  = stats->getChannelFieldName(channel, field);
                    const char* unit  = stats->getChannelFieldUnit(channel, field);
                    Serial.printf("%s = %0.2f %s\r\n", name, value, unit);
                }
            }
    
            Serial.println();
        }
    }
    Serial.println();
}

void setupWiFi() {
    Serial.print("Connecting WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("connected");
}

void setupNTP() {
    configTime(0, 0, NTP_SERVER);
    setenv("TZ", TIME_ZONE, 1);
    tzset();
}

void setup() {
    Serial.begin(115200);
    setupWiFi();
    setupNTP();
    setupHoymiles();
    setupInverter();
}

void loop() {
    Hoymiles.loop();
    printStatistics();
}