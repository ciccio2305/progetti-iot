#include <Arduino.h>
#include <WiFi.h>
#include <string>
#include "painlessmesh.h"

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

//Scheduler sched; //scheduler azioni semaforo
WiFiClient client; 
int turni=0;

// void handle_semaphore();
// Task taskSemaphore(TASK_MILLISECOND * 1, TASK_FOREVER, &handle_semaphore);

// void handle_semaphore(){
//   if (client.available()) {
//         String c = client.readString();
//         Serial.print("ricevuto:");
//         Serial.println(c.c_str());

//         if(strstr(c.c_str(),"master")!=NULL){
//           Serial.println("messaggio di presentazione");
//         }
//         else if(strstr(c.c_str(),"rosso")!=NULL){
//           Serial.println("divento rosso");
//           client.println("ok");
//           redOn(0);
//         }
//         else if(strstr(c.c_str(),"verde")!=NULL){
//           Serial.println("divento verde");
//           client.println("ok");
//           greenOn(0);
//         }
//         else if(strstr(c.c_str(),"giallo")!=NULL){
//           Serial.println("divento giallo");
//           client.println("ok");
//           yellowOn(0);
//         }
//         turni=0;
//       }
// }

int status = WL_IDLE_STATUS;

WiFiServer server(80);

bool send_msg_to_server(WiFiClient dest,const char* msg){
  int counter=0;
  while(counter<10){
    if(dest.connected()){
      Serial.print("invio msg:");
      Serial.println(msg);
      
      dest.println(msg);
      return true;
    }else{
      counter++;
      delay(50);
    }
 }
 Serial.println("msg failed");
 return false;
}


void setup() {
  // inizializzazione della seriale <-> baudrate 115200
  Serial.begin(115200);

  // inizializzazione delle GPIO
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  //manageWiFiEvents();
  WiFi.mode(WIFI_STA);
  status = WiFi.begin("semaforo1", "semaforo1234");

  bool flag=true;
  yellowOn(0);
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    
    if(flag){
      yellowOn(0);
      flag=!flag;
    }else{
      all_off(0);
      flag=!flag;
    }
    delay(500);
    // don't do anything else:
  }


  Serial.println(WiFi.localIP());

  server.begin();
  client.connect(WiFi.gatewayIP(),80);

  // sched.init();
  // sched.addTask( taskSemaphore );
  // sched.enableAll();
  
  //taskSemaphore.enable();
  
  send_msg_to_server(client,id);
  all_off(0);
  
  Serial.println("setup finita");
}

void loop() {
  //stampa il messaggio che il client mi invia 
  //WiFiClient client2 = server.available();
  //sched.execute();
  
  if (client.available()) {
        String c = client.readString();
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
        turni=0;
      }
  
  if(turni==3000000){
    Serial.println("il master si è rotto");
    
    bool flag=true;
    yellowOn(0);
    while ( WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      WiFi.begin("semaforo1", "semaforo1234");
      if(flag){
        yellowOn(0);
        flag=!flag;
      }else{
        all_off(0);
        flag=!flag;
      }
      delay(500);
      // don't do anything else:
    }
    client.connect(WiFi.gatewayIP(),80);
    send_msg_to_server(client,id);
    turni=0;
  }
  turni++;
}