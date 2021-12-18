#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h> 

int R_Led_pin = 12;
int G_Led_pin = 12;
int B_Led_pin = 12;

float Sender=12; // Sender ID 10 for TX 11 for RX1 12 for RX2
float IGN_Status_1=0;
float IGN_Status_2=0;
float IGN_FIRE_1=0;
float IGN_FIRE_2=0;
float safety_word=6969; 


#define SEND_TIMEOUT 100 
#define SLEEP_SECS 5 //5 sec

volatile boolean callbackCalled;

#define WIFI_CHANNEL 4
//uint8_t remoteMac_1[] = {0xA0, 0x20, 0xA6, 0x13, 0x60, 0xA8};
uint8_t remoteMac_1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t remoteMac_2[] = {0xA0, 0x20, 0xA6, 0x13, 0x60, 0xA8};


struct __attribute__((packed)) MESSAGE {
  float Sender; // Sender ID 10 for TX 11 for RX1 12 for RX2
  float IGN_Status;
  float IGN_FIRE;
  float safety_word; 
} messageData;




void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}



void initEspNow() {
  if (esp_now_init()!=0) {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(remoteMac_1, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  esp_now_add_peer(remoteMac_2, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {

    char macStr[18];
    formatMacAddress(mac, macStr, 18);
    memcpy(&messageData, data, sizeof(messageData));

    Serial.print("recv_cb, msg from device: "); 
    Serial.println(macStr);

    if (messageData.safety_word==safety_word) {
      if(messageData.Sender==11) {
        Serial.print("Odbiornik 1 Status: ");
        Serial.print(messageData.IGN_Status);
        IGN_Status_1=messageData.IGN_Status;
      }
      if(messageData.Sender==12) {
        Serial.print("Odbiornik 2 Status: ");
        Serial.print(messageData.IGN_Status);
        IGN_Status_2=messageData.IGN_Status;
      }
    }
    else {
      Serial.println("Wrong safety word");
    }
  });

  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("send_cb, send done, status = %i\n", sendStatus);
    callbackCalled = true;
  });
}


void sendAll() {
uint8_t bs[sizeof(messageData)];
  memcpy(bs, &messageData, sizeof(messageData));
  esp_now_send(NULL, bs, sizeof(messageData)); // NULL means send to all peers

} 

void sendTo(uint8_t* mac) {
uint8_t bs[sizeof(messageData)];
  memcpy(bs, &messageData, sizeof(messageData));
  esp_now_send(mac, bs, sizeof(messageData)); // NULL means send to all peers

} 


void setup() {
  delay(100);
  
  //PIN init
  pinMode(R_Led_pin, OUTPUT);
  analogWrite(R_Led_pin, 0);
  pinMode(G_Led_pin, OUTPUT);
  analogWrite(G_Led_pin, 255);
  pinMode(B_Led_pin, OUTPUT);
  analogWrite(B_Led_pin, 255);
  Serial.begin(115200);

  //ESP-NOW Init

  WiFi.mode(WIFI_STA); // Station mode for esp-now sensor node
  WiFi.disconnect();

  Serial.printf("This mac: %s, ", WiFi.macAddress().c_str()); 
  Serial.printf("target mac 1: %02x%02x%02x%02x%02x%02x", remoteMac_1[0], remoteMac_1[1], remoteMac_1[2], remoteMac_1[3], remoteMac_1[4], remoteMac_1[5]); 
  Serial.printf("target mac 2: %02x%02x%02x%02x%02x%02x", remoteMac_2[0], remoteMac_2[1], remoteMac_2[2], remoteMac_2[3], remoteMac_2[4], remoteMac_2[5]); 
  Serial.printf(", channel: %i\n", WIFI_CHANNEL); 

  initEspNow();

  
}






void loop() {





}


