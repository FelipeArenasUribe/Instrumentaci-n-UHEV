#include <SoftwareSerial.h> //Para crear un purto serial por Software en pines digitales

//Declarar Comunicacion serial del GPS por Software
#define RxPin 5
#define TXPin 6
SoftwareSerial gps(5, 6);

//Declarar pines de sensores
#define pinLlanta 2 //Pin de interrupción de sensor hall para la Llanta
#define pinMotor 3 //Pin de interrupción de sensor hall para el motor
#define pinVoltaje A0 //Pin del sensor de Voltaje
#define pinCorriente A1 //Pin del sensor de Corriente 

//Declarar Variables Globales

//Variables GPS
char gpsin = ' ';
String stringgps = "$";// as the code looks for the '$'
// it needs to be added manually
// again to the string
// SET DESIRED SENTENCE IDENTIFIER HERE, set "$" only for all sentences
String sentence1 = "$GPRMC";

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
bool newdataRPMmotor = false;
bool newdataRPMLlanta = false;

//Funciones de las interrupciones

//PIT Timestamp
ISR(TIMER1_COMPA_vect) {
  mseg++;
}

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
  Serial.begin(9600);
  gps.begin(9600);

  cli();
  //Generar Interrupcion Timer 1 para TimeStamp
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

  sei();
}


void loop() {
  GetVI();
  //Print data from sensors
  printVI();
  printRPMLlanta();
  printRPMMotor();

  if (gps.available() > 0) { // if there is data coming into the serial line
    gpsin = gps.read(); // get a byte of data
  }
  if (gpsin == 36) { //ascii code '$'
    GetLineGPS();
  }
}

void printVI() {
  Serial.print("VI,");//Imprimir periodo de la Llanta
  Serial.print(t_stamp);
  Serial.print(",");
  Serial.print(voltaje);
  Serial.print(",");
  Serial.println(corriente);

}

void printRPMLlanta() {
  if (newdataRPMLlanta == true) {
    Serial.print("LLanta, ");//Imprimir periodo de la Llanta
    Serial.print(t0);
    Serial.print(",");
    Serial.println(delta_t);
    newdataRPMLlanta = false;
  }
}

void printRPMMotor() {
  if (newdataRPMmotor == true) {
    Serial.print("Motor, ");//Imprimir periodo de la Llanta
    Serial.print(t_init);
    Serial.print(",");
    Serial.println(periodo);
    newdataRPMmotor = false;
  }
}


void GetLineGPS() {
  while (gpsin != 13) //ascii code 'return', end of sentence
  {
    if (gps.available() > 0) // if there is data coming into the serial line
    {
      gpsin = gps.read(); // get next the byte of data while gpsin != 13
      stringgps += gpsin;
    }
  }
  if (stringgps.startsWith(sentence1)) {
    Serial.print(stringgps); // when gpsin != 13 NOT true (gpsin == 13) string to serial
    Serial.print(',');
    Serial.println(mseg);
  }
  stringgps = "$"; //empty string for next sentence
}

void GetVI() {
  t_stamp = mseg;
  voltaje = analogRead(pinVoltaje); //lectura del voltaje
  corriente = analogRead(pinCorriente); //Lectura de la corriente
}
