#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/* ===== WiFi ===== */
const char* ssid     = "RedmiNote9";
const char* wifiPass = "123SSS123@";

/* ===== ThingsBoard MQTT ===== */
const char* mqttServer = "mqtt.thingsboard.cloud";
const int   mqttPort   = 1883;
const char* tbToken    = "JrWrtyovgwc9zgjadLrB";  // Device access token

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

/* ================= MQTT RECONNECT ================= */
void reconnectMQTT()
{
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard MQTT...");
    if (client.connect("ESP8266Client", tbToken, NULL)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

/* ================= SETUP ================= */
void setup()
{
  Serial.begin(115200);
  delay(2000);

  WiFi.begin(ssid, wifiPass);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
}

/* ================= LOOP ================= */
void loop()
{
  if (!client.connected())
    reconnectMQTT();

  client.loop();

  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    line.trim();

    // Expecting:
    // lux,lamp,soil,pump,mq135,temp,hum,fan
    int idx[8];
    int pos = 0;

    for (int i = 0; i < 7; i++) {
      idx[i] = line.indexOf(',', pos);
      if (idx[i] < 0) return;   // invalid packet
      pos = idx[i] + 1;
    }

    String lux        = line.substring(0, idx[0]);
    String lamp       = line.substring(idx[0] + 1, idx[1]);
    String soil       = line.substring(idx[1] + 1, idx[2]);
    String pump       = line.substring(idx[2] + 1, idx[3]);
    String mq135      = line.substring(idx[3] + 1, idx[4]);
    String temp       = line.substring(idx[4] + 1, idx[5]);
    String hum        = line.substring(idx[5] + 1, idx[6]);
    String fan        = line.substring(idx[6] + 1);

    if (millis() - lastPublish >= 5000)
    {
      String payload =
        "{"
        "\"light_intensity\":" + lux +
        ",\"led_status\":" + lamp +
        ",\"soil_moisture\":" + soil +
        ",\"pump_status\":" + pump +
        ",\"mq135_adc\":" + mq135 +
        ",\"temperature\":" + temp +
        ",\"humidity\":" + hum +
        ",\"fan_status\":" + fan +
        "}";

      Serial.print("Publishing: ");
      Serial.println(payload);

      client.publish("v1/devices/me/telemetry", payload.c_str());

      lastPublish = millis();
    }
  }
}
