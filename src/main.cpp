#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h> 

#define R_Led_pin 16
#define G_Led_pin 12
#define B_Led_pin 14
#define IGN_pin 15
#define RX1_pin 13
#define RX2_pin 2


int IGN_Status_1=0; // 0 - disconected 1-no igniter 2-Ready unarmed 3-Ready ARMED
int IGN_Status_2=0;
int IGN_FIRE_1=0;
int IGN_FIRE_2=0;


#define SEND_TIMEOUT 100 
#define SLEEP_SECS 5 //5 sec

volatile boolean callbackCalled;

#define WIFI_CHANNEL 4
//uint8_t remoteMac_1[] = {0xA0, 0x20, 0xA6, 0x13, 0x60, 0xA8};
uint8_t remoteMac_1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t remoteMac_2[] = {0xA0, 0x20, 0xA6, 0x13, 0x60, 0xA8};
char macBuf[18];

struct __attribute__((packed)) MESSAGE {
  int Sender ;
  int IGN_Status = 0;
  int IGN_FIRE = 0;
} messageData;




void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void LED(int R, int G, int B) {
  analogWrite(R_Led_pin, R);
  analogWrite(G_Led_pin, G);
  analogWrite(B_Led_pin, B);
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


    
  });

  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    Serial.printf("send_cb, send done, status = %i\n", sendStatus);
    callbackCalled = true;
  });
}


void sendAll() {
  messageData.Sender = 100;
uint8_t bs[sizeof(messageData)];
  memcpy(bs, &messageData, sizeof(messageData));
  esp_now_send(NULL, bs, sizeof(messageData)); // NULL means send to all peers

} 

void sendTo(uint8_t* mac) {
  messageData.Sender = 101;
uint8_t bs[sizeof(messageData)];
  memcpy(bs, &messageData, sizeof(messageData));
  esp_now_send(mac, bs, sizeof(messageData)); 

} 


void setup() {
  delay(100);
  
  //PIN init
  pinMode(IGN_pin, INPUT);
  pinMode(RX1_pin, INPUT_PULLUP);
  pinMode(RX2_pin, INPUT_PULLUP);
  pinMode(R_Led_pin, OUTPUT);
  pinMode(G_Led_pin, OUTPUT);
  pinMode(B_Led_pin, OUTPUT);
  LED(0,0,0);
   Serial.begin(115200);

  //ESP-NOW Init

  WiFi.mode(WIFI_STA); // Station mode for esp-now sensor node
  WiFi.disconnect();

  Serial.printf("This mac: %s, \n", WiFi.macAddress().c_str()); 
  Serial.printf("target mac 1: %02x%02x%02x%02x%02x%02x\n", remoteMac_1[0], remoteMac_1[1], remoteMac_1[2], remoteMac_1[3], remoteMac_1[4], remoteMac_1[5]); 
  Serial.printf("target mac 2: %02x%02x%02x%02x%02x%02x\n", remoteMac_2[0], remoteMac_2[1], remoteMac_2[2], remoteMac_2[3], remoteMac_2[4], remoteMac_2[5]); 
  Serial.printf(", channel: %i\n", WIFI_CHANNEL); 
  formatMacAddress(remoteMac_1, macBuf, 18);
  Serial.println(macBuf);
  initEspNow();

  
}






void loop() {
  if(digitalRead(RX2_pin)||digitalRead(RX1_pin)) {
    LED(0,255,255);
  }
  if(!digitalRead(RX1_pin)) {
    messageData.IGN_Status = 1;
    sendAll();
    LED(255,0,255);
  }

  if(!digitalRead(RX2_pin)) {
    messageData.IGN_Status = 2;
    sendTo(remoteMac_1);
    LED(255,255,0);
  }

  if(digitalRead(IGN_pin)) {
    messageData.IGN_Status = 3;
    sendTo(remoteMac_2);
    LED(255,0,0);
  }
  delay(500);
}


