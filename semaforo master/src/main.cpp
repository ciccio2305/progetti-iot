#include <Arduino.h>
#include <WiFi.h>
#include <string>
#include "painlessmesh.h"

#define id "3"

#define RED_LED 3
#define YELLOW_LED 4
#define GREEN_LED 0

#define DELAY_3SECONDS 3000
#define DELAY_1_5SECONDS 1500

#define TEMPO_VERDE TASK_MILLISECOND*4000
#define TEMPO_GIALLO TASK_MILLISECOND*2000
#define TEMPO_TUTTI_ROSSI TASK_MILLISECOND*500

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


Scheduler sched;
void handle_semaphore();
Task taskSemaphore(TASK_MILLISECOND * 1, TASK_FOREVER, &handle_semaphore);

WiFiServer server(80);

WiFiClient client;

enum stati{
  VERDE,
  GIALLO,
  ROSSO,
  ALTRI_VERDE,
  ALTRI_GIALLO,
  ALTRI_ROSSO
};

stati stato= VERDE;

bool send_msg_to_server(WiFiClient dest,const char* msg){
  delay(200);
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

  WiFi.mode(WIFI_AP);
  WiFi.softAP("semaforo1", "semaforo1234");
  greenOn(0);

  sched.init();
  sched.addTask( taskSemaphore );
  taskSemaphore.enable();

  server.begin();
}

WiFiClient peer[3];
bool map_peer[3]={false,false,false};

int peer_connessi=0;
int turni=0;
void loop() {

  client=server.available();

  if (client) {
    Serial.println("New Client.");

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
        
        if(map_peer[id_]){
          Serial.println("peer gia presente lo sostituisco con quello nuovo");
          peer_connessi--;
        }
        map_peer[id_]=true;
        peer[id_]=client;
        peer_connessi++;
        send_msg_to_server(client,"ciao sono il master");
        send_msg_to_server(client,"diventa rosso");
        break; 
      }
    }
    Serial.println("Client aggiunto.");
  }


  sched.execute();
}

void handle_semaphore(){
  //taskSemaphore.disable();
  bool status_last=false;
  

  switch (stato){
    
    case VERDE:{
      Serial.println("VERDE");
      if(map_peer[1]){
        status_last=send_msg_to_server(peer[1],"diventa verde");
      }
      if(!status_last){
        map_peer[1]=false;
        peer_connessi--;
      }
      greenOn(0);

      stato=GIALLO;

      taskSemaphore.delay(TEMPO_VERDE);//tempo verde
      break;
    }
      
    case GIALLO:{
      Serial.println("giallo");
      if(map_peer[1]){
        //peer[1].readString();
        status_last=send_msg_to_server(peer[1],"diventa giallo");
        // while(!peer[1].available() && status_last){
        //   delay(50);
        // }
        // peer[1].readString();
      }
        
      if(!status_last){
        map_peer[1]=false;
        peer_connessi--;
      }
      
      yellowOn(0);

      stato=ROSSO;

      taskSemaphore.delay(TEMPO_GIALLO);//tempo giallo

      break;
    }
    
    case ROSSO:{
      Serial.println("rosso");

      if(map_peer[1]){
        //peer[1].readString();
        status_last=send_msg_to_server(peer[1],"diventa rosso");
        // while(!peer[1].available() && status_last){
        //   delay(50);
        // }
        // peer[1].readString();
      }
        
      if(!status_last){
        map_peer[1]=false;
        peer_connessi--;
      }
      
      //delay(1000);
      
      redOn(0);
      
      stato=ALTRI_VERDE;

      taskSemaphore.delay(TEMPO_TUTTI_ROSSI); //forse non serve
      break;
    }
    
    case ALTRI_VERDE:{
      Serial.println("invio agli altri di diventare verde");
      
      if(map_peer[0])
        status_last=send_msg_to_server(peer[0],"diventa verde");

      if(!status_last){
        map_peer[0]=false;
        peer_connessi--;
      }

      status_last=false;
      if(map_peer[2])
        status_last=send_msg_to_server(peer[2],"diventa verde");
      if(!status_last){
        map_peer[2]=false;
        peer_connessi--;
      }
      
      stato=ALTRI_GIALLO;

      taskSemaphore.delay(TEMPO_VERDE);//tempo verde secondo
      break;
    }

    case ALTRI_GIALLO:{
       Serial.println("invio agli altri di diventare gialli");

        if(map_peer[0]){
          status_last=send_msg_to_server(peer[0],"diventa giallo");
        }

        if(!status_last){
          map_peer[0]=false;
          peer_connessi--;
        }
        
        status_last=false;
        if(map_peer[2])
        status_last=send_msg_to_server(peer[2],"diventa giallo");
        if(!status_last){
          map_peer[2]=false;
          peer_connessi--;
        }

        stato=ALTRI_ROSSO;
      
        taskSemaphore.delay(TEMPO_GIALLO);//tempo giallo secondo


        break;
    }

    case ALTRI_ROSSO:{
      
      Serial.println("invio agli altri di diventare Rosso");

      if(map_peer[0])
      status_last=send_msg_to_server(peer[0],"diventa rosso");
      if(!status_last){
        map_peer[0]=false;
        peer_connessi--;
      }

      status_last=false;
      if(map_peer[2])
      status_last=send_msg_to_server(peer[2],"diventa rosso");
      if(!status_last){
        map_peer[2]=false;
        peer_connessi--;
      }
      
      stato=VERDE;

      taskSemaphore.delay(TEMPO_TUTTI_ROSSI);//forse non serve
      break;
    } 
  }


}