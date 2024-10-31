#include <algorithm>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <math.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "Wi-Fi name";
const char* password = "Password";
String serverName = "http://emrealp.net/update-sensor/up.php";
//String notificationUrl = ; // 

unsigned long lastTime = 0;
const int wakeUpInterval = 5 * 1000000ul; // Wake up interval in microseconds (5 seconds)

const int s0 = D1;
const int s1 = D2;
const int s2 = D3;
const int s3 = D4;
const int SIG_pin = A0;

const float R1 = 1000;           // Resistor value in ohms
const float VCC = 3.3;            // Voltage
const float MUX_RESISTANCE = 70;  // Multiplexer resistance in ohms

const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;  // Steinhart-Hart coefficients for converting analog values to kelvin

// calculating temperature from ADC value
float calculateTemperature(int Vo) {
    float voltage = (Vo * VCC) / 1023.0;
    float R2 = (R1 * (VCC / voltage - 1.0)) - MUX_RESISTANCE;
    float logR2 = log(R2);
    float tKelvin = 1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2);
    return tKelvin - 273.15;  // Convert Kelvin to Celsius
}

// Array of clusters, thermistors are clustered by two pairs
const int clusters[6][2] = {
    {0, 1}, // Cluster 1: 
    {1, 2}, // Cluster 2: 
    {2, 3}, // Cluster 3: 
    {3, 4}, // Cluster 4: 
    {4, 5}, // Cluster 5: 
    {5, 6}  // Cluster 6: 
};

void setup() {
    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);
    digitalWrite(s0, LOW);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);
    Serial.begin(115200);

    connectWiFi();

    float temperatures[7]; 

    // Read temperatures from channels
    for (int i = 0; i < 7; i++) {
        int Vo = readMux(i);
        temperatures[i] = calculateTemperature(Vo);
    }

   
    temperatures[0] += 5.0;
    temperatures[1] -= 2.0;
    temperatures[5] -= 5.0;

    // Outlier detection
    for (int i = 0; i < 7; i++) {
        if (isOutlier(temperatures, i)) {
            if (checkSurrounding(i, temperatures)) {
                Serial.print("Outlier detected at channel ");
                Serial.print(i);
                Serial.print(": ");
                Serial.print(temperatures[i]);
                Serial.println(" C");

                // sending notification if there's an outlier detected
                sendOutlierNotification(i, temperatures[i]);
            } else {
                Serial.print("False outlier at channel ");
                Serial.print(i);
                Serial.println(" - ignored due to surrounding values.");
            }
        }
    }

    // sending sensor data to the website
    sendSensorData(temperatures);

    WiFi.disconnect();
    Serial.println("Going to sleep...");
    delay(1000);        
    WiFi.disconnect();  
    delay(100);         

    
    Serial.println("Going into Deep Sleep");
    ESP.deepSleep(6e7);
}

void connectWiFi() {
    Serial.println();
    delay(500);
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected to WiFi!");
}

void loop() {
    // storing temperature readings
}

int readMux(int channel) {
    int controlPin[] = { s0, s1, s2, s3 };
    int muxChannel[16][4] = {
        { 0, 0, 0, 0 }, { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 1, 1, 0, 0 }, { 0, 0, 1, 0 }, { 1, 0, 1, 0 }, { 0, 1, 1, 0 }, { 1, 1, 1, 0 }, { 0, 0, 0, 1 }, { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 1, 1, 0, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 0, 1, 1, 1 }, { 1, 1, 1, 1 }
    };
    for (int i = 0; i < 4; i++) {
        digitalWrite(controlPin[i], muxChannel[channel][i]);
    }
    return analogRead(SIG_pin);  // read analog value from selected channel
}

bool isOutlier(float temperatures[], int index) {
    float sum = 0.0;
    float mean;
    float stdDev = 0.0;
    int n = 7;  // number of sensors + 1

    for (int i = 0; i < n; i++) {
        sum += temperatures[i];
    }
    mean = sum / n;

    for (int i = 0; i < n; i++) {
        stdDev += pow(temperatures[i] - mean, 2);
    }
    stdDev = sqrt(stdDev / n);

    float zScore = (temperatures[index] - mean) / stdDev;

    // With 95% certainty, accept Z scores greater than 1.65 as outliers
    return abs(zScore) > 1.6448;
}

bool checkSurrounding(int index, float temperatures[]) {
    for (int i = 0; i < 6; i++) {  
        if (index == clusters[i][0] || index == clusters[i][1]) {
            int otherIndex = (index == clusters[i][0]) ? clusters[i][1] : clusters[i][0];
            if (isOutlier(temperatures, otherIndex)) {
                return false;  // If the other thermistor in the cluster also reads an outlying value, reject it.
            }
        }
    }
    return true;
}

void sendOutlierNotification(int channel, float temperature) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        http.begin(client, notificationUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String notificationData = "channel=" + String(channel) + "&temperature=" + String(temperature);
        int httpResponseCode = http.POST(notificationData);

        Serial.print("Notification HTTP Response code: ");
        Serial.println(httpResponseCode);

        http.end();
    } else {
        Serial.println("WiFi Disconnected - Notification not sent");
    }
}

void sendSensorData(float temperatures[]) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        http.begin(client, serverName);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String httpRequestData = "s1=" + String(temperatures[0]) + "&s2=" + String(temperatures[1]) + "&s3=" + String(temperatures[2]) + "&s4=" + String(temperatures[3]) + "&s5=" + String(temperatures[4]) + "&s6=" + String(temperatures[5]) + "&s7=" + String(temperatures[6]);
        int httpResponseCode = http.POST(httpRequestData);

        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        http.end();
    } else {
        Serial.println("WiFi Disconnected - Data not sent");
    }
}
