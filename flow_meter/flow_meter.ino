#define BPS 115200 //Velocidad de las comunicaciones

# define PENDIG false
# define EXECUTED true

#include <SoftwareSerial.h>
#define RXPIN 6
#define TXPIN 5
SoftwareSerial wifiSerialInit (RXPIN,TXPIN);

int value = 0; // valor de cualquier pin digital

// se han reseteado los contadores al estado inicial?
bool conters_init_state;


struct counter
   {  String name;  
      int pin ;
      int state ; 
      unsigned long pulse;
      unsigned long time_output ;
      unsigned long time_transmision ;
   } ;
   
counter e1, e2, e3, e4, e5, e6;

counter *counters[] = {&e1, &e2, &e3, &e4, &e5, &e6};

// tiempo base a partir del que se configuran las tx ca 5 sgs
unsigned long time_base_tx ; 

//declaración de funciones
void test_transmision(counter *);
void configure_time_base_tx();
void counters_init();

void setup() {
  
  Serial.begin(BPS);   //iniciar puerto serie
  wifiSerialInit.begin(BPS);  // puerto serie al esp12

  e1.name = "e1";
  e1.pin = 14;
  e1.state = 2;
  e1.pulse= 0 ;
  e1.time_output = 10000 ;
  e1.time_transmision = 20000 ;

  e2 = e1 ;
  e2.pin = 15 ;
  e2.name = "e2";
  
  e3 = e1 ;
  e3.pin = 16 ;
  e3.name = "e3";

  
  e4 = e1 ;
  e4.pin = 17 ;
  e4.name = "e4";
    
  e5 = e1 ;
  e5.pin = 18 ;
  e5.name = "e5";
    
  e6 = e1 ;
  e6.pin = 19 ;
  e6.name = "e6";

  time_base_tx = 0;
  
  pinMode(e1.pin, INPUT_PULLUP);  //definir pin como entrada
  pinMode(e2.pin, INPUT_PULLUP);  //definir pin como entrada
  pinMode(e3.pin, INPUT_PULLUP);  //definir pin como entrada
  pinMode(e4.pin, INPUT_PULLUP);  //definir pin como entrada  
  pinMode(e5.pin, INPUT_PULLUP);  //definir pin como entrada
  pinMode(e6.pin, INPUT_PULLUP);  //definir pin como entrada
}
 
void loop(){
   
/************************************/
/***********  cron  *******************/
/************************************/
  if ((conters_init_state == PENDIG) && (millis() < 10000)) { // resenteo los tiempos de espera
    counters_init();
  }

  // inhibir la transmisión durante el último minuto
  // evita configurar tiempos de tx que excedan el tiempo maximo
  if ((millis() > time_base_tx + 60000) && (millis() < 4294867295)){ 
    configure_time_base_tx(); 
  }
  
  //configuro la variable durante el último segundo
  if ((millis() > 4294866295)){ 
    conters_init_state = PENDIG ;
  }


/************************************/
/***********  e_n  ******************/
/************************************/
  // Estado de los contadores:
  //    2: Alto, esperando bajo
  //    3: transición a bajo
  //    4: Bajo, esperando alto
  //    5 :transicion a alto
  
  for (int n=0; n < 6; n++){
    switch (counters[n]->state) {           
      case 2: // bajo?
        value = digitalRead(counters[n]->pin);  //lectura digital de pin
        if (value == LOW) {
          counters[n]->pulse++;
          counters[n]->state=3;
          counters[n]->time_output = millis() + 50;
          if(millis() > 4294967195) {counters[n]->time_output = 4294967295;}
        }
        else {test_transmision(counters[n]);}
        break;
      case 3: //transicion bajo
        if (millis() >= counters[n]->time_output ) {
          counters[n]->state=4;
        }  
        break;
      case 4: // alto?
        value = digitalRead(counters[n]->pin);  //lectura digital de pin
        if (value == HIGH) {
          counters[n]->state = 5;
          counters[n]->time_output = millis() + 50;
          if(millis() > 4294967195) {counters[n]->time_output = 4294967295;}
        }
        else {test_transmision(counters[n]);} 
        break;       
      case 5: //transicion alto
        if (millis() >= counters[n]->time_output ) {
          counters[n]->state=2;
        }     
        break;            
      default:
        Serial.println("error: ningun case");
        break;
      }
  }
} // final loop()

/************************************/
/****** definicion de funciones *****/
/************************************/

void test_transmision(counter *e){
  String message_to_tx = "";
  if (millis() >= e->time_transmision ) {
    message_to_tx += (e->name);
    message_to_tx += ":";
    message_to_tx += String(e->pulse);
    Serial.println(message_to_tx);
    wifiSerialInit.println (message_to_tx);
    
    e->time_transmision =  4294967295;
  }
}

void configure_time_base_tx(){  
  time_base_tx = millis() ;
  e1.time_transmision = time_base_tx + 5000  ;
  e2.time_transmision = time_base_tx + 10000 ;
  e3.time_transmision = time_base_tx + 15000 ;
  e4.time_transmision = time_base_tx + 20000 ;
  e5.time_transmision = time_base_tx + 25000 ;
  e6.time_transmision = time_base_tx + 30000 ;  
}

void counters_init(){
  e1.state = 2;
  e2.state = 2;
  e3.state = 2;
  e4.state = 2;
  e5.state = 2;
  e6.state = 2;
  conters_init_state = EXECUTED;
}
