#include <SoftwareSerial.h> //Para crear un purto serial por Software en pines digitales
/*
//Declarar Comunicacion serial de la SD por Software
#define RxPin 7
#define TXPin 6
SoftwareSerial serialSD(7, 6); //Rx, Tx
*/
//Declarar comunicacion serial del GPS por Software
#define RxPinGPS 4
#define TxPinGPS 5
SoftwareSerial serialGPS(4, 5); //Rx, Tx

//Declarar pines de sensores
#define pinLlanta 2 //Pin de interrupción de sensor hall para la Llanta
#define pinMotor 3 //Pin de interrupción de sensor hall para el motor
#define pinVoltaje A0 //Pin del sensor de Voltaje
#define pinCorriente A1 //Pin del sensor de Corriente 

//---------Declarar Variables Globales---------//

//Variables GPS
char gpsin = ' ';
String stringgps = "$";
// SET DESIRED SENTENCE IDENTIFIER HERE, set "$" only for all sentences
String identifier = "$GPRMC";

//Variables Voltaje Corriente
volatile uint16_t voltaje = 0; //Inicializa la variable para medición de voltaje
volatile uint16_t corriente = 0; //Inicializa la variable para medición de corriente

//Variables RPMLlanta
volatile unsigned long delta_t = 0; //Periodo
volatile unsigned long t0 = 0; //Tiempo primer flanco de bajada
volatile bool PrimeraRev = true; //Identifica que es la primera revolucion de la llanta

//Variables RPMMotor
volatile uint8_t pulsos = 0;
volatile unsigned long t_init;
volatile unsigned long x;
volatile unsigned long periodo;

//Variables Time Stamp
volatile unsigned long mseg = 0; //Contador Timestamp
volatile unsigned long t_stamp; //Timestamp

//New Data Identifiers
bool newdataVI = false;
bool newdataRPMmotor = false;
bool newdataRPMLlanta = false;

//---------Funciones de las interrupciones---------//

//PIT Timestamp
ISR(TIMER1_COMPA_vect) {
  mseg++;
}

//PIT Voltaje Corriente
/*
ISR(TIMER0_COMPA_vect){//timer0 interrupt 2kHz
  t_stamp = mseg;
  voltaje = analogRead(pinVoltaje); //lectura del voltaje
  corriente = analogRead(pinCorriente); //Lectura de la corriente
  newdataVI = true;
}
*/

ISR(TIMER2_OVF_vect) {
  t_stamp = mseg;
  voltaje = analogRead(pinVoltaje); //lectura del voltaje
  corriente = analogRead(pinCorriente); //Lectura de la corriente
  newdataVI = true;
  //Serial.println("VI,"+t_stamp+','+voltaje+','+corriente); //Imprimir lecturas de voltaje y corriente
}

//---------Funciones lectura RPM---------//
//Periodo RPMLlanta
void rpmllanta() {
  if (PrimeraRev == true) {
    t0 = mseg;
    PrimeraRev = false;
  }
  else if (PrimeraRev == false) {
    delta_t = mseg - t0;
    t0 = mseg;
    newdataRPMLlanta = true;
  }
}

//Periodo RPMMotor
void rpmmotor() {   // Funcion que se ejecuta durante cada interrupion
  pulsos++;
  x = mseg;
  if (pulsos == 1) {
    t_init = x;
  }
  if (pulsos == 6) {
    periodo = x - t_init;
    t_init = x;
    newdataRPMmotor = true;
  }
  if (pulsos == 6) {
    pulsos = 1;
  }
}

void setup() {
  //Iniciar puertos de comunicacion serial
  Serial.begin(9600); //Inicializacion comunicacion serial con SD
  serialGPS.begin(9600); //Inicializacion comunicacion serial con GPS

  //Iniciar Interrupciones
  cli();
  //Generar Interrupcion Timer 1 para TimeStamp cada 1ms
  TCNT1 = 0;
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 250;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= 0b00000011; //62500 ---> 110
  TIMSK1 |= 1 << OCIE1A;

  //Interrupcion rpm Llanta
  attachInterrupt(digitalPinToInterrupt(pinLlanta), rpmllanta, FALLING);
  //Interrupcion rpm Motor
  attachInterrupt(digitalPinToInterrupt(pinMotor), rpmmotor, FALLING);
  /*
    //set timer0 interrupt at 1kHz
    TCCR0A = 0;// set entire TCCR2A register to 0
    TCCR0B = 0;// same for TCCR2B
    TCNT0  = 0;//initialize counter value to 0
    // set compare match register for 2khz increments
    OCR0A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256)
    // turn on CTC mode
    TCCR0A |= (1 << WGM01);
    // Set CS01 and CS00 bits for 64 prescaler
    TCCR0B |= (1 << CS01) | (1 << CS00);
    // enable timer compare interrupt
    TIMSK0 |= (1 << OCIE0A);
  */
  //Crear PIT para lectura de Voltaje y Corriente
  SREG = (SREG & 0b01111111); //Desabilitar interrupciones
  TCNT2 = 0; //Limpiar contenido del registro del Timer-2
  TIMSK2 = TIMSK2 | 0b00000001; //Habilita la interrupcion por desbordamiento
  TCCR2B = 0b00000011; //Configura preescala para que FT2 sea de 250KHz
  SREG = (SREG & 0b01111111) | 0b10000000; //Habilitar interrupciones
  
  sei();
}


void loop() {
  printVI();
  printRPMLlanta();
  printRPMMotor();
  
  if (serialGPS.available() > 0) { // if there is data coming into the serial line
    gpsin = serialGPS.read(); // get a byte of data
  }
  if (gpsin == 36) { //ascii code '$'
    GetLineGPS();
  }
}

//Funcion para imprimir datos de VI
void printVI() {
  if (newdataVI == true) {
    Serial.print("VI,");//Imprimir periodo de la Llanta
    Serial.print(t_stamp);
    Serial.print(",");
    Serial.print(voltaje);
    Serial.print(",");
    Serial.println(corriente);
    newdataVI = false;
  }
}

//Funcion para imprimir datos de RPM Llanta
void printRPMLlanta() {
  if (newdataRPMLlanta == true) {
    Serial.print("LLanta, ");//Imprimir periodo de la Llanta
    Serial.print(t0);
    Serial.print(",");
    Serial.println(delta_t);
    newdataRPMLlanta = false;
  }
}

//Funcion para imprimir datos de RPM Motor
void printRPMMotor() {
  if (newdataRPMmotor == true) {
    Serial.print("Motor, ");//Imprimir periodo de la Llanta
    Serial.print(t_init);
    Serial.print(",");
    Serial.println(periodo);
    newdataRPMmotor = false;
  }
}

//Funcion para lectura de V y I
void GetVI() {
  t_stamp = mseg;
  voltaje = analogRead(pinVoltaje); //lectura del voltaje
  corriente = analogRead(pinCorriente); //Lectura de la corriente
  newdataVI = true;
}

void GetLineGPS() {
  while (gpsin != 13) //ascii code 'return', end of sentence
  {
    if (serialGPS.available() > 0) // if there is data coming into the serial line
    {
      gpsin = serialGPS.read(); // get next the byte of data while gpsin != 13
      stringgps += gpsin;
    }
  }
  if (stringgps.startsWith(identifier)) {
    Serial.print(stringgps); // when gpsin != 13 NOT true (gpsin == 13) string to serial
    Serial.print(',');
    Serial.println(mseg);
  }
  stringgps = "$"; //empty string for next sentence
}
