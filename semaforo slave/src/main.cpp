#include <Arduino.h>
#include <WiFi.h>
#include <string>

const char* id ="2";

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

int status = WL_IDLE_STATUS;

WiFiClient client;
WiFiServer server(80);

bool send_msg_to_server(IPAddress dest,const char* msg){
  WiFiClient client_sender;
  client_sender.connect( dest, 80);
  delay(200);
  int counter=0;
  while(counter<10){
    if(client_sender.connected()){
      Serial.print("invio msg:");
      Serial.println(msg);

      client_sender.println(msg);
      client_sender.stop();
      return true;
    }else{
      counter++;
      delay(50);
    }
 }
 client_sender.stop();
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
  
  Serial.println("server avviato");
  send_msg_to_server(WiFi.gatewayIP(),id);
  all_off(0);
  Serial.println("setup finita");
}


int turni=0;

void loop() {
  //stampa il messaggio che il client mi invia 
  WiFiClient client2 = server.available();
  if (client2) {
    Serial.println(client2.remoteIP());
    Serial.println("New Client");
    while (client2.connected()){
      
      if(client2.available()){
        Serial.println();
        String c = client2.readString();
        Serial.print("ricevuto:");
        Serial.println(c.c_str());

        if(strstr(c.c_str(),"master")!=NULL){
          Serial.println("messaggio di presentazione");
        }
        else if(strstr(c.c_str(),"rosso")!=NULL){
          Serial.println("divento rosso");
          redOn(0);
        }
        else if(strstr(c.c_str(),"verde")!=NULL){
          Serial.println("divento verde");
          greenOn(0);
        }
        else if(strstr(c.c_str(),"giallo")!=NULL){
          Serial.println("divento giallo");
          yellowOn(0);
        }
        turni=0;
        break;
      }
      else{
        Serial.print(".");
        delay(100);
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
  if(turni==1000000){
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
    send_msg_to_server(WiFi.gatewayIP(),id);
    turni=0;
  }
  turni++;
}