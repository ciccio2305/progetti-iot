#include <Arduino.h>
#include <WiFi.h>
#include <string>

#define id "3"

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

void redOn(uint16_t del) 
{
  Serial.printf("Red light on for %.2f seconds\n", (float)del/1000);
  ledOnLedsOff(RED_LED, YELLOW_LED, GREEN_LED, del);
}

void yellowOn(uint16_t del)
{
  Serial.printf("Yellow light on for %.2f seconds\n", (float)del/1000);
  ledOnLedsOff(YELLOW_LED, RED_LED, GREEN_LED, del);
}

void greenOn(uint16_t del)
{
  Serial.printf("Green light on for %.2f seconds\n", (float)del/1000);
  ledOnLedsOff(GREEN_LED, RED_LED, YELLOW_LED, del);
}


WiFiServer server(80);

WiFiClient client;

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

  WiFi.mode(WIFI_AP);
  WiFi.softAP("semaforo1", "semaforo1234");
  greenOn(0);

  server.begin();
  
}

IPAddress* peer[3]={nullptr,nullptr,nullptr};

int peer_connessi=0;
int turni=0;
void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        String c = client.readString();
        Serial.println(c.c_str());
        int id_=c.c_str()[0]-48;
        Serial.print("ricevuto id:");
        Serial.println(id_);
        if(id_<0||id_>2){
          Serial.println("sconosciuto");
          break;
        }
        if(peer[id_]!=nullptr){
          Serial.println("peer gia presente lo sostituisco con quello nuovo");
          peer_connessi--;
        }
        peer[id_]=new IPAddress(client.remoteIP());
        peer_connessi++;
        send_msg_to_server(client.remoteIP(),"ciao sono il master");
        send_msg_to_server(client.remoteIP(),"diventa rosso");
        break; 
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }

  bool status_last=false;
  if(turni==1){
    Serial.println("verde");
    if(peer[1]!=nullptr)
      status_last=send_msg_to_server(*peer[1],"diventa verde");
    if(!status_last){
      peer[1]=nullptr;
      peer_connessi--;
    }
    delay(1100);
    greenOn(0);
  }
  
  if(turni==200000){
    Serial.println("giallo");
    if(peer[1]!=nullptr)
    status_last=send_msg_to_server(*peer[1],"diventa giallo");
    if(!status_last){
      peer[1]=nullptr;
      peer_connessi--;
    }
    delay(1100);
    yellowOn(0);
  }

  if(turni==300000){
    Serial.println("rosso");
    if(peer[1]!=nullptr)
    status_last=send_msg_to_server(*peer[1],"diventa rosso");
    if(!status_last){
      peer[1]=nullptr;
      peer_connessi--;
    }
    delay(1100);
    redOn(0);
  }

  if(turni==350000){

    if(peer[0]!=nullptr)
    status_last=send_msg_to_server(*peer[0],"diventa verde");
    if(!status_last){
      peer[0]=nullptr;
      peer_connessi--;
    }
      
    if(peer[2]!=nullptr)
    status_last=send_msg_to_server(*peer[2],"diventa verde");
    if(!status_last){
      peer[2]=nullptr;
      peer_connessi--;
    }
  }

  if(turni==550000){
    if(peer[0]!=nullptr)
    status_last=send_msg_to_server(*peer[0],"diventa giallo");
    if(!status_last){
      peer[0]=nullptr;
      peer_connessi--;
    }
      
    if(peer[2]!=nullptr)
    status_last=send_msg_to_server(*peer[2],"diventa giallo");
    if(!status_last){
      peer[2]=nullptr;
      peer_connessi--;
    }

  }

  if(turni==650000){
    if(peer[0]!=nullptr)
    status_last=send_msg_to_server(*peer[0],"diventa rosso");
    if(!status_last){
      peer[0]=nullptr;
      peer_connessi--;
    }
  
    if(peer[2]!=nullptr)
    status_last=send_msg_to_server(*peer[2],"diventa rosso");
    if(!status_last){
      peer[2]=nullptr;
      peer_connessi--;
    }
      
  }

  if(turni==700000){
    Serial.println("reset");
    turni=0;
  }


  turni++;
  
  //semaphore();
}
