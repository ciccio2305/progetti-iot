#include <Arduino.h>
#include <WiFi.h>
#include <string>
#include "painlessmesh.h"
#include "BluetoothSerial.h"

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


enum stati{
  VERDE,
  GIALLO,
  ROSSO,
  ALTRI_VERDE,
  ALTRI_GIALLO,
  ALTRI_ROSSO
};

stati stato= VERDE;

BluetoothSerial SerialBT;

bool send_msg_to_server(uint8_t* dest,const char* msg){
  bool status=SerialBT.connect(dest);
  if(status){
    while(!SerialBT.connected()){
    }

    SerialBT.println(msg);
    delay(200);
    SerialBT.disconnect();
  } 
  return status;
}

bool SlaveConnected=false;
int recatt=0;

void Bt_Status(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_OPEN_EVT) {
    Serial.println ("Client Connected");
    SlaveConnected = true;
    recatt = 0;
  }
  else if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("Client Disconnected");
    SlaveConnected = false;
  }
}
String name="uno";

void setup() {
  // inizializzazione della seriale <-> baudrate 115200
  Serial.begin(115200);

  // inizializzazione delle GPIO
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  //manageWiFiEvents();

  sched.init();
  sched.addTask( taskSemaphore );
  taskSemaphore.enable();

  SerialBT.register_callback(Bt_Status);
  SerialBT.begin(name, true);
  
  Serial.printf("The device \"%s\" started in master mode, make sure slave BT device is on!\n", name.c_str());
 
}

bool map_peer[3]={true,true,true};
u_int8_t peer[3][6]={ {0x4c,0x75,0x25,0xa7,0x7a,0x08},
                      {0x64,0xB7,0x08,0x8C,0x3E,0xAA},
                      {0x64,0xB7,0x08,0x8C,0x3B,0x52}};

int turni=0;
void loop() {
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
        delay(1100);
      }
      if(!status_last){
        map_peer[1]=false;
      }
      greenOn(0);

      stato=GIALLO;

      taskSemaphore.delay(TEMPO_VERDE);
      break;
    }
      
    case GIALLO:{
      Serial.println("giallo");
      if(map_peer[1]){
        status_last=send_msg_to_server(peer[1],"diventa giallo");
        delay(1100);
      }
        
      if(!status_last){
        map_peer[1]=false;
      }
      
      yellowOn(0);

      stato=ROSSO;

      taskSemaphore.delay(TEMPO_GIALLO);//tempo giallo

      break;
    }
    
    case ROSSO:{
      Serial.println("rosso");

      if(map_peer[1]){
        status_last=send_msg_to_server(peer[1],"diventa rosso");
        delay(1100);
      }
        
      if(!status_last){
        map_peer[1]=false;
      }
      
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
      }

      status_last=false;
      if(map_peer[2])
        status_last=send_msg_to_server(peer[2],"diventa verde");
      if(!status_last){
        map_peer[2]=false;
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
        }
        
        status_last=false;
        if(map_peer[2])
        status_last=send_msg_to_server(peer[2],"diventa giallo");
        if(!status_last){
          map_peer[2]=false;
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
      }

      status_last=false;
      if(map_peer[2])
      status_last=send_msg_to_server(peer[2],"diventa rosso");
      if(!status_last){
        map_peer[2]=false;
      }
      
      stato=VERDE;

      taskSemaphore.delay(TEMPO_TUTTI_ROSSI);//forse non serve
      break;
    } 
  }


}