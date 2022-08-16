#define UIP_CONF_UDP             1
#include <UIPEthernet.h>
#include "PubSubClient.h"
#define CLIENT_ID       "OK1"
#define PUBLISH_DELAY   2000
//String ip = "192.168.137.1";
byte ip[] = {192, 168, 2, 100};
byte mac[] = {0x34, 0x6F, 0x24, 0xBE, 0x8E, 0x77};


EthernetClient ethClient;
PubSubClient mqttClient;
//DHT dht(DHTPIN, DHTTYPE);

long previousMillis;

void setup() {

  // setup serial communication
  Serial.begin(9600);
  Serial.println("OK");
  Ethernet.begin(mac,ip);
  Serial.println("OK2");
  // setup mqtt client
  mqttClient.setClient(ethClient);
  mqttClient.setServer("broker.hivemq.com", 1883);
  Serial.println("MQTT client configured");
  previousMillis = millis();
}

void loop() {

  // Condition for stop or shutdown
  if (millis() - previousMillis > PUBLISH_DELAY) {
    Serial.println("Condition");
    sendData();
    previousMillis = millis();
  }
  mqttClient.loop();
}

void sendData() {

  if (mqttClient.connect(CLIENT_ID)) {
     Serial.println("Send");
     mqttClient.publish("telemm/stop","STOP");
     }
}

char * deblank(char *str) {
  char *out = str;
  char *put = str;

  for (; *str != '\0'; ++str) {

    if (*str != ' ') {
      *put++ = *str;
    }
  }
  *put = '\0';
  return out;
}
