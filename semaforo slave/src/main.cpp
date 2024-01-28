#include <Arduino.h>
#include <WiFi.h>
#include <string>
#include "painlessmesh.h"

#include "BluetoothSerial.h"

const char* id ="1";

#define RED_LED 3
#define YELLOW_LED 4
#define GREEN_LED 0

#define DELAY_3SECONDS 3000
#define DELAY_1_5SECONDS 1500

typedef enum {
  IDLE,
  RED,
  YELLOW,
  GREEN
} m_state;

m_state state = IDLE;

void ledOnLedsOff(uint8_t high, uint8_t low1, uint8_t low2, uint16_t del)
{
  digitalWrite(high, HIGH);
  digitalWrite(low1, LOW);
  digitalWrite(low2, LOW);
  delay(del);
}
void all_leds_off(uint8_t high, uint8_t low1, uint8_t low2, uint16_t del)
{
  digitalWrite(high, LOW);
  digitalWrite(low1, LOW);
  digitalWrite(low2, LOW);
  delay(del);
}
void all_off(uint16_t del){
  all_leds_off(RED_LED, YELLOW_LED, GREEN_LED, del);
}
void redOn(uint16_t del) 
{
  //Serial.printf("Red light on for %.2f seconds\n", (float)del/1000);
  ledOnLedsOff(RED_LED, YELLOW_LED, GREEN_LED, del);
}

void yellowOn(uint16_t del)
{
  //Serial.printf("Yellow light on for %.2f seconds\n", (float)del/1000);
  ledOnLedsOff(YELLOW_LED, RED_LED, GREEN_LED, del);
}

void greenOn(uint16_t del)
{
  //Serial.printf("Green light on for %.2f seconds\n", (float)del/1000);
  ledOnLedsOff(GREEN_LED, RED_LED, YELLOW_LED, del);
}

int turni=0;

// void handle_semaphore();

int status = WL_IDLE_STATUS;

bool MasterConnected;

void Bt_Status (esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

  if (event == ESP_SPP_SRV_OPEN_EVT) { 
    Serial.println ("Client Connected");
    MasterConnected = true;
  }
  else if (event == ESP_SPP_CLOSE_EVT ) {
    Serial.println ("Client Disconnected");
    MasterConnected = false;
  }
}

BluetoothSerial SerialBT;

void setup() {
  // inizializzazione della seriale <-> baudrate 115200
  Serial.begin(115200);

  // inizializzazione delle GPIO
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  
  SerialBT.register_callback(Bt_Status);
  String name="due";
  SerialBT.begin(name);
  Serial.println(SerialBT.getBtAddressString());
}

void loop() {
  //stampa il messaggio che il client mi invia 
  //WiFiClient client2 = server.available();
  //sched.execute();
  if (SerialBT.available()) {
        String c = SerialBT.readString();
        Serial.print("ricevuto:");
        Serial.println(c.c_str());

        if(strstr(c.c_str(),"master")!=NULL){
          Serial.println("messaggio di presentazione");
        }
        else if(strstr(c.c_str(),"rosso")!=NULL){
          Serial.println("divento rosso");
          //client.println("ok");
          redOn(0);
        }
        else if(strstr(c.c_str(),"verde")!=NULL){
          Serial.println("divento verde");
          //client.println("ok");
          greenOn(0);
        }
        else if(strstr(c.c_str(),"giallo")!=NULL){
          Serial.println("divento giallo");
          //client.println("ok");
          yellowOn(0);
        }
      }
}