#define Analog0 A0
#define Analog1 A1
#define Analog2 A2
#define Analog3 A3

#include <PCD8544.h>
float referencia = 4.84; // tensión máxima de las señales
float V0, V1, V2, V3;
PCD8544 pantalla;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Puerto serial   
  analogReference(referencia);
  
  pantalla.begin();
}

void loop() {
  pantalla.setPower(1);
   
  // put your main code here, to run repeatedly:
  V0 = map(analogRead(Analog0), 0, 1023,-24,24);   // leer entrada analógica y transformarla al rango original
  V1 = map(analogRead(Analog1), 0, 1023,-24,24); 
  V2 = map(analogRead(Analog2), 0, 1023,-24,24); 
  V3 = map(analogRead(Analog3), 0, 1023,-24,24); 

  pantalla.setCursor(0, 1);
  pantalla.print("V0 ="); pantalla.print(V0);

  pantalla.setCursor(0, 2);
  pantalla.print("V1 ="); pantalla.print(V1);

  pantalla.setCursor(0, 3);
  pantalla.print("V2 ="); pantalla.print(V2);

  pantalla.setCursor(0, 4);
  pantalla.print("V3 ="); pantalla.print(V2);
}
