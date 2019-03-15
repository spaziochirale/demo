/***************************************************
 *    ESEMPIO IMPIEGO ANN PRE-ADDESTRATE
 *        Riconoscimento dei colori
 *    
 *    Author: Stefano Capezzone
 *    Version: 1.0
 *    Corso Professionale di arduino Avanzato
 *    
 */

// Utilizzo della libreria Neurona
#include <Neurona.h>
#include "ssd1306.h"
#include "nano_gfx.h"
#include "sova.h"



#define NET_INPUTS 3
#define NET_OUTPUTS 10
#define NET_LAYERS 2

#define LED_DELAY 500
#define LDR_PIN A0

int rgb[] = {8, 9, 10}; //led pins
int input[] = {0, 0, 0}; //RGB values
double netInput[] = {-1.0, 0.0, 0.0, 0.0}; // il primo valore è sempre -1 per il "bias"
int calib[3][2] = {{329, 778}, {166, 569}, {140, 528}}; // inizializzo i valori di calibrazione

char *colors[] = {"NERO", "ROSSO", "VERDE", "BLU", "GIALLO", "MARRONE", "VIOLA", "ARANCIO", "ROSA", "BIANCO"};

// Topologia della rete, 1 hidden layer da 6 neuroni, 10 output
int layerSizes[] = {6, NET_OUTPUTS, -1}; 


// Array con i Pesi 
double PROGMEM const initW[] = {2.753086,-11.472257,-3.311738,16.481226,19.507006,20.831778,7.113330,-6.423491,1.907215,6.495393,-27.712126,26.228203,-0.206367,-5.724560,-22.278070,30.065610,6.139262,-10.814282,28.513130,-9.784946,6.467021,0.055005,3.730361,4.145092,2.479019,0.013003,-3.582416,-16.364391,14.133357,-5.089288,1.637492,5.894826,1.415764,-3.315533,14.814289,-20.906571,-1.568656,1.917658,4.910184,4.039419,-10.848469,-5.641680,-4.132432,10.711442,3.759935,19.507702,17.728724,-3.210244,-2.476992,8.988450,5.196827,2.636043,17.357207,2.005429,11.713386,-5.453253,-6.940325,10.752005,0.666605,-7.266082,-3.587120,-9.921817,-12.682059,-15.456143,-13.740927,0.508265,15.179410,-11.143178,-19.085120,1.251235,22.006491,-4.227328,-0.444516,3.589025,0.649661,13.675598,-13.026884,-11.229070,-15.300703,-1.718191,6.737973,-28.176802,-2.505471,5.197970,7.007983,-2.869269,3.650349,18.029204,4.098356,10.481188,-2.566311,9.927770,2.344936,4.524327};

// Creo la Rete Neurale Multi Layer Perceptron, attivation Logistic, pesi nella PROGMEM
MLP mlp(NET_INPUTS,NET_OUTPUTS,layerSizes,MLP::LOGISTIC,initW,true);

volatile int readPressed=0; // Quando si preme il pulsante di lettura viene posto a 1 dalla ISR
volatile int calibPressed=0; // Quando si preme il pulsante di calibrazione viene posto a 1 dalla ISR

// Definizione delle ISR
void readPressedISR(){
  readPressed=1;
}
void calibPressedISR(){
  calibPressed=1;
}


void setup() {
  // installazione delle ISR
  attachInterrupt(digitalPinToInterrupt(2),calibPressedISR,FALLING); // Calib su PIN 2 in PULLUP
  attachInterrupt(digitalPinToInterrupt(3),readPressedISR,FALLING);  // Read Color su PIN 3 in PULLUP
  
  ssd1306_128x64_i2c_init();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen();
  ssd1306_printFixed(0,  8, "Premere CALIBRA oppure", STYLE_NORMAL);
  ssd1306_printFixed(0,  16, "avvicinare un oggetto", STYLE_NORMAL);
  ssd1306_printFixed(0,  24, "e premere LEGGI COLORE", STYLE_NORMAL);
  
  Serial.begin(9600);
  Serial.println("Color Sensor Regognition using Multi Layer Perceptron");
  Serial.println("Premere il pulsante CALIBRA oppure esporre un colore e premere LETTURA COLORE");
  for(int i=0;i<3;i++){
    pinMode(rgb[i], OUTPUT);
    digitalWrite(rgb[i], HIGH);
  }
  
}

void loop() {
  if(calibPressed) {
    delay(200);
    calibPressed=0;
    calibrate();
  }
  if(readPressed) {
    delay(200);
    readPressed=0;
    readColor();
  }

}

// Definisco la funzione per effettuare la calibrazione
void calibrate(){
  Serial.println("Esporre il NERO e premere il pulsante di LETTURA COLORE");
  ssd1306_clearScreen();
  ssd1306_printFixed(0,  8, "Esporre il NERO", STYLE_NORMAL);
  ssd1306_printFixed(0,  16, "Premere Lettura", STYLE_NORMAL);
  while (!readPressed); // loop finché readPressed è zero
  delay(200);
  readPressed=0; // reset stato readPressed
  for(int i=0;i<3;i++){
    digitalWrite(rgb[i], LOW); 
    delay(LED_DELAY);
    calib[i][0] = analogRead(LDR_PIN);
    digitalWrite(rgb[i], HIGH);
    delay(LED_DELAY);
  }
  Serial.println("Esporre il BIANCO e premere il pulsante di LETTURA COLORE");
  ssd1306_clearScreen();
  ssd1306_printFixed(0,  8, "Esporre il BIANCO", STYLE_NORMAL);
  ssd1306_printFixed(0,  16, "Premere Lettura", STYLE_NORMAL);
  while (!readPressed); // loop finché readPressed è zero
  delay(200);
  readPressed=0;
  for(int i=0;i<3;i++){
    digitalWrite(rgb[i], LOW); 
    delay(LED_DELAY);
    calib[i][1] = analogRead(LDR_PIN);
    digitalWrite(rgb[i], HIGH);
    delay(LED_DELAY);
  }

  Serial.println("Valori di calibrazione:");
  for(int i=0;i<3;i++){
    Serial.print("{");
    Serial.print(calib[i][0]);
    Serial.print(",");
    Serial.print(calib[i][1]);
    Serial.print("}");
  }
  Serial.println("");
  ssd1306_clearScreen();
  ssd1306_printFixed(0,  8, "Calibrazione Ok!", STYLE_NORMAL);
  
}

void readColor(){
  Serial.println("Lettura Colore...");
  delay(100);
  readPressed=0;
  for(int i=0;i<3;i++){
    digitalWrite(rgb[i], LOW); 
    delay(LED_DELAY);
    input[i] = analogRead(LDR_PIN);
    input[i] = map(input[i], calib[i][0], calib[i][1], 0, 255);
    input[i] = input[i]<0?0:input[i]>255?255:input[i]; // Elimino eventuali fuori scala
    digitalWrite(rgb[i], HIGH); 
    delay(LED_DELAY);
  }
  Serial.print("Valori RGB misurati: ");
  for(int i=0;i<3;i++){
      Serial.print(input[i]);
      if(i<2)
        Serial.print(" / ");
      netInput[i+1] = (double)input[i]/255.0;
    }
    Serial.println("");
    Serial.println("Identifico il colore...");
    Serial.println(colors[mlp.getActivation(netInput)]);
    ssd1306_clearScreen();
    ssd1306_printFixed(0,  8, colors[mlp.getActivation(netInput)], STYLE_NORMAL);

}
