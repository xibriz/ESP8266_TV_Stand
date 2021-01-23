/**
 * Boilerplate for ESP8266 connecting to MQTT
 * with the possibility to update firmware OTA
 */

#include "secrets.h" 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>

WiFiClient espClient;
PubSubClient client(espClient);

/**
 * Publish new firmwares like this:
 * mosquitto_pub -t 'esp/boilerPlate/fw/update' -m 'http://localhost/new_firmware.bin'
 */
const String mqtt_topic_base = "esp/tvstand";
const String mqtt_topic_fota = "fw/update";
const String mqtt_topic_fw_version = "fw/version";
const String mqtt_topic_fw_info = "fw/info";
const String FW_VERSION = "1001";

const String mqtt_topic_up = "up";
const String mqtt_topic_down = "down";

// NB Using pins that are high on boot
const int8 pin_up = 4; // D2
const int8 pin_down = 5; // D1

const char* msg_ok = "1";

// float lastPublishTime = 0;
// const float publishInterval = 5000; //Publish to MQTT every 5 seconds

void setup() {
  setupWiFi();
  setupMqtt();

  pinMode(pin_up, OUTPUT);      // set pin to input
  digitalWrite(pin_up, LOW);   // turn on pullup resistors

  pinMode(pin_down, OUTPUT);    // set pin to input
  digitalWrite(pin_down, LOW); // turn on pullup resistors

  Serial.begin (115200);
  delay(10);
}

void loop() {
  loopMqtt();
  
  // // Publish to MQTT every X seconds
  // if(millis() - lastPublishTime >= publishInterval) {
  //   lastPublishTime = millis();
  //   // Publish some values
  //   client.publish(createTopic("hello").c_str(), ((String)"Hello MQTT").c_str());
  // }
  
  // /**
  //  * INSERT YOUR LOGIC HERE
  //  */
  // Serial.println("INSERT YOUR LOGIC HERE");
  delay(10);
}

/**
 * Set up WiFi
 */
void setupWiFi()
{
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(secret_ssid, secret_ssid_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

/**
 * Set up MQTT
 */
void setupMqtt()
{
  client.setServer(secret_mqtt_host, secret_mqtt_port);
  client.setCallback(callback);
}

/**
 * MQTT callback function that runs when information is sendt on a topic
 * that we subscribe to
 */
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  // If we recieve anything on the firmware update topic topic
  if (strcmp(topic, createTopic(mqtt_topic_fota).c_str()) == 0) {
    for (int i = 0; i < length; i++) {
      msg.concat( (char)payload[i] );
    }
    // Check for new FW
    checkForUpdates(msg);
  }

  // If we recieve anything on the up topic
  if (strcmp(topic, createTopic(mqtt_topic_up).c_str()) == 0) {
    for (int i = 0; i < length; i++) {
      msg.concat( (char)payload[i] );
    }
    // Bring the TV up
    up(msg);
  }

  // If we recieve anything on the up topic
  if (strcmp(topic, createTopic(mqtt_topic_down).c_str()) == 0) {
    for (int i = 0; i < length; i++) {
      msg.concat( (char)payload[i] );
    }
    // Bring the TV down
    down(msg);
  }
  Serial.println(msg);
}

/**
 * Ensure the MQTT lirary gets some attention too
 * (Used for subscribe)
 */
void loopMqtt()
{
  if (!client.connected()) {
    reconnectMqtt();
  }
  client.loop();
  delay(100);
}

/**
 * Check that we are still connected, if not, reconnect
 */
void reconnectMqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // IMPORTANT! Create a random client ID
    // If the client ID is not unique subscribe will not work!
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.subscribe(createTopic(mqtt_topic_fota).c_str()); // Subscribe on FW updates
      client.subscribe(createTopic(mqtt_topic_up).c_str()); // Subscribe
      client.subscribe(createTopic(mqtt_topic_down).c_str()); // Subscribe
      // Publish FW version on boot
      client.publish(createTopic(mqtt_topic_fw_version).c_str(), FW_VERSION.c_str());
    }
    else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**
 * Download new FW from the URL that we recieved on the firmware update topic
 */
void checkForUpdates(String fwURL) {
  String mqtt_topic_fw_info_full = createTopic(mqtt_topic_fw_info);

  client.publish(mqtt_topic_fw_info_full.c_str(), ((String)"Checking for firmware updates on:").c_str());
  client.publish(mqtt_topic_fw_info_full.c_str(), fwURL.c_str());

  t_httpUpdate_return ret = ESPhttpUpdate.update( fwURL );

  switch(ret) {
    case HTTP_UPDATE_FAILED:
      client.publish(mqtt_topic_fw_info_full.c_str(), ((String)HTTP_UPDATE_FAILED).c_str());
      client.publish(mqtt_topic_fw_info_full.c_str(), ((String)ESPhttpUpdate.getLastError()).c_str());
      client.publish(mqtt_topic_fw_info_full.c_str(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      client.publish(mqtt_topic_fw_info_full.c_str(), ((String)HTTP_UPDATE_NO_UPDATES).c_str());
      break;
  }
}

/**
 * Combines the base topic with additional topic information
 */
String createTopic(String topic) {
  return (mqtt_topic_base+((String)"/"+topic));
}

void up(String msg) {
  Serial.println("UP");
  if (strcmp(msg.c_str(), msg_ok) == 0) {
    Serial.println("UP OK");
    digitalWrite(pin_up, HIGH);
    delay(1000);
    digitalWrite(pin_up, LOW);
  }
}

void down(String msg) {
  Serial.println("DOWN");
  if (strcmp(msg.c_str(), msg_ok) == 0) {
    Serial.println("DOWN OK");
    digitalWrite(pin_down, HIGH);
    delay(1000);
    digitalWrite(pin_down, LOW);
  }
}