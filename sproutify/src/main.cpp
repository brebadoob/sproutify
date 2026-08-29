/*
 * Sproutify — Team Planet11
 * Board: LilyGO T-Display (same board as lab4_cloud)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
#define WIFI_SSID     "insert ssid here"
#define WIFI_PASSWORD "insert password here5"

// SAS token from Azure IoT Explorer 
#define SAS_TOKEN     "SharedAccessSignature sr=cs147group11IoTHub.azure-devices.net%2Fdevices%2F147esp32&sig=sEXYU0apOXJsMKyvX%2BDks8fRclcbrQOYiwLy5LBv5uY%3D&se=1788058854"

// Azure IoT configuration
const char* IOT_HUB_NAME = "cs147group11IotHub";
const char* DEVICE_ID    = "147esp32";

// Wiring on ESP32
#define SOIL_MOISTURE_PIN 32
#define RED_PIN           27
#define YELLOW_PIN        26
#define GREEN_PIN         25
#define BUZZER_PIN        22

// Certificate Azure needs for HTTPS
const char* root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
    "MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n"
    "MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n"
    "UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n"
    "eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n"
    "GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n"
    "mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n"
    "SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n"
    "dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n"
    "AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n"
    "VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n"
    "buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n"
    "AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n"
    "b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n"
    "Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n"
    "oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n"
    "dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n"
    "AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n"
    "klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n"
    "kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n"
    "eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n"
    "penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n"
    "QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n"
    "q/xKzj3O9hFh/g==\n"
    "-----END CERTIFICATE-----\n";


// Moisture percent cutoffs
// used by the LEDs and buzzer
#define UNDERWATERED_THRESHOLD         20
#define WATER_NEEDED_THRESHOLD         40
#define OVERWATERED_THRESHOLD          80
#define SEVERELY_OVERWATERED_THRESHOLD 90

// Raw probe values for "fully dry" and "fully wet"
#define DRY_ADC 1337
#define WET_ADC 0

#define SENSOR_INTERVAL        1000
#define TELEMETRY_INTERVAL     3000
#define OVERWATER_BLINK_SLOW   1000
#define OVERWATER_BLINK_FAST    300
#define BUZZER_HOLD_MS         3000

// Time (in seconds) wait before buzzing
#define PLANT_UNDERWATERED          1
#define PLANT_WATER_NEEDED          2
#define PLANT_HEALTHY               3
#define PLANT_OVERWATERED           4
#define PLANT_SEVERELY_OVERWATERED  5

int plantState = PLANT_HEALTHY;
int moistureLevel = 0;
int lastRawAdc = 0;
unsigned long lastSensorTime = 0;
unsigned long lastTelemetryTime = 0;
unsigned long blinkTimer = 0;
unsigned long alertStartTime = 0;
bool redBlinkState = false;
bool alertArmed = false;

int getSoilMoisture();
void determinePlantState();
void updateActuators();
void connectWiFi();
void sendTelemetry();
String getPlantStatus();
String telemetryUrl();

void setup() {
    Serial.begin(9600);
    delay(1000);
    Serial.println();
    Serial.println("Sproutify boot (LilyGO T-Display)");

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    pinMode(SOIL_MOISTURE_PIN, INPUT);
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    connectWiFi();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    if (millis() - lastSensorTime >= SENSOR_INTERVAL) {
        lastSensorTime = millis();
        moistureLevel = getSoilMoisture();
        determinePlantState();
        updateActuators();

        Serial.print("ADC=");
        Serial.print(lastRawAdc);
        Serial.print("  Moisture=");
        Serial.print(moistureLevel);
        Serial.print("%  Status=");
        Serial.println(getPlantStatus());
    }

    if (millis() - lastTelemetryTime >= TELEMETRY_INTERVAL) {
        lastTelemetryTime = millis();
        sendTelemetry();
    }
}
// Read GPIO 32 and turn the raw 0–4095 value into 0–100% moisture
// probe outputs a large number when dry and a small number when wet,
// map dry to 0% and wet to 100%
int getSoilMoisture() {
    lastRawAdc = analogRead(SOIL_MOISTURE_PIN);
    int moisture = map(lastRawAdc, DRY_ADC, WET_ADC, 0, 100);
    return constrain(moisture, 0, 100);
}

// Turn moisture percent into one of five plant states
// starts a 3-second timer when it first enters a state that should buzz
void determinePlantState() {
    int previous = plantState;

    if (moistureLevel < UNDERWATERED_THRESHOLD) {
        plantState = PLANT_UNDERWATERED;
    } else if (moistureLevel < WATER_NEEDED_THRESHOLD) {
        plantState = PLANT_WATER_NEEDED;
    } else if (moistureLevel < OVERWATERED_THRESHOLD) {
        plantState = PLANT_HEALTHY;
    } else if (moistureLevel < SEVERELY_OVERWATERED_THRESHOLD) {
        plantState = PLANT_OVERWATERED;
    } else {
        plantState = PLANT_SEVERELY_OVERWATERED;
    }

    bool needsBuzzer =
        (plantState == PLANT_UNDERWATERED) ||
        (plantState == PLANT_SEVERELY_OVERWATERED);

    if (needsBuzzer && previous != plantState) {
        alertStartTime = millis();
        alertArmed = true;
    }
    if (!needsBuzzer) {
        alertArmed = false;
        digitalWrite(BUZZER_PIN, LOW);
    }
}

// Drive the LEDs and the buzzer from the current plant state
// green = good, yellow = water soon, red = dry, blinking red = too wet
// buzzer only after the plant has been dry or soaked for 3 seconds
void updateActuators() {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    if (plantState == PLANT_UNDERWATERED) {
        digitalWrite(RED_PIN, HIGH);
        if (alertArmed && (millis() - alertStartTime >= BUZZER_HOLD_MS)) {
            digitalWrite(BUZZER_PIN, HIGH);
        }
    } else if (plantState == PLANT_WATER_NEEDED) {
        digitalWrite(YELLOW_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
    } else if (plantState == PLANT_HEALTHY) {
        digitalWrite(GREEN_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
    } else if (plantState == PLANT_OVERWATERED) {
        digitalWrite(BUZZER_PIN, LOW);
        if (millis() - blinkTimer >= OVERWATER_BLINK_SLOW) {
            blinkTimer = millis();
            redBlinkState = !redBlinkState;
        }
        digitalWrite(RED_PIN, redBlinkState);
    } else if (plantState == PLANT_SEVERELY_OVERWATERED) {
        if (millis() - blinkTimer >= OVERWATER_BLINK_FAST) {
            blinkTimer = millis();
            redBlinkState = !redBlinkState;
        }
        digitalWrite(RED_PIN, redBlinkState);
        if (alertArmed && (millis() - alertStartTime >= BUZZER_HOLD_MS)) {
            digitalWrite(BUZZER_PIN, HIGH);
        }
    }
}

// Joins the Wi-Fi
// retries every 20s if SSID/password is incorrect
void connectWiFi() {
    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - start > 20000) {
            Serial.println();
            Serial.println("Wi-Fi timeout, retrying");
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            start = millis();
        }
    }

    Serial.println();
    Serial.println("Wi-Fi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

// Status labels sent to Azure
String getPlantStatus() {
    switch (plantState) {
        case PLANT_UNDERWATERED:         return "Underwatered";
        case PLANT_WATER_NEEDED:         return "Water Needed Soon";
        case PLANT_HEALTHY:              return "Healthy";
        case PLANT_OVERWATERED:          return "Overwatered";
        case PLANT_SEVERELY_OVERWATERED: return "Severely Overwatered";
        default:                         return "Unknown";
    }
}

// Azure IoT Hub address it's posted to
String telemetryUrl() {
    String url = "https://";
    url += IOT_HUB_NAME;
    url += ".azure-devices.net/devices/";
    url += DEVICE_ID;
    url += "/messages/events?api-version=2020-03-13";
    return url;
}

// Pack latest reading as JSON and HTTPS POST it to IoT Hub
// 204 = success, 401 = bad token, 404 = wrong hub/device name, -1 = no network/TLS
void sendTelemetry() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi down; telemetry skipped");
        return;
    }

    JsonDocument doc;
    doc["deviceId"] = DEVICE_ID;
    doc["soilMoisture"] = moistureLevel;
    doc["rawAdc"] = lastRawAdc;
    doc["plantStatus"] = getPlantStatus();
    doc["plantState"] = plantState;
    doc["uptimeMs"] = millis();

    char buffer[320];
    serializeJson(doc, buffer, sizeof(buffer));

    WiFiClientSecure client;
    client.setCACert(root_ca);

    HTTPClient http;
    http.begin(client, telemetryUrl());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", SAS_TOKEN);

    int httpCode = http.POST(buffer);

    if (httpCode == 204) {
        Serial.print("Telemetry OK: ");
        Serial.println(buffer);
    } else {
        Serial.print("Telemetry failed HTTP ");
        Serial.print(httpCode);
        Serial.print(" ");
        Serial.println(http.errorToString(httpCode));
        Serial.println(buffer);
    }

    http.end();
}
