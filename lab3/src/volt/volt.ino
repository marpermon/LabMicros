#define ANALOG0 A0
#define ANALOG1 A1
#define ANALOG2 A2
#define ANALOG3 A3
#define ACDC 8
#define FRECUENCIA 60
#define SAMPLES 1000


#include <PCD8544.h>
double referencia = 4.84; // tensión máxima de las señales
double V0, V1, V2, V3;
double SquareV0 = 0.0, SquareV1 = 0.0, SquareV2 = 0.0, SquareV3 = 0.0;

PCD8544 pantalla;

void imprimir_lcd(char str[15], double V); // funcion para imprimir en la pantalla
void puerto_serial(double V0, double V1, double V2, double V3); // funcion para imprimir en el puerto serial

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Puerto serial   
  analogReference(referencia);
  pinMode(ACDC, INPUT);
  pantalla.begin();
}

void loop() {


switch (digitalRead(ACDC)) {
  case HIGH: // En DC
    V0 = map(analogRead(ANALOG0), 0, 1023,-24,24);   // leer entrada analógica y transformarla al rango original
    V1 = map(analogRead(ANALOG1), 0, 1023,-24,24); 
    V2 = map(analogRead(ANALOG2), 0, 1023,-24,24); 
    V3 = map(analogRead(ANALOG3), 0, 1023,-24,24); 
    pantalla.setCursor(0, 1); // Coloca lo que se va a imprimir en una posición enl a pantalla
    imprimir_lcd("V0 =", V0); // Imprime en la pantalla

    pantalla.setCursor(0, 2);
    imprimir_lcd("V1 =", V1);   

    pantalla.setCursor(0, 3);
    imprimir_lcd("V2 =", V2);

    pantalla.setCursor(0, 4);
    imprimir_lcd("V3 =", V3);
    
    puerto_serial(V0, V1, V2, V3); // Imprime en ele puerto serial
    

  break;

  case LOW: //En AC
    for (int i = 0; i < SAMPLES; i++) { 
    /* Usamos 100 samples en un periodo para hacer la una suma de cuadrados
    aproximada, segun la formula del valor RMS*/
        V0 = map(analogRead(ANALOG0), 0, 1023,-24,24);   
        V1 = map(analogRead(ANALOG1), 0, 1023,-24,24); 
        V2 = map(analogRead(ANALOG2), 0, 1023,-24,24); 
        V3 = map(analogRead(ANALOG3), 0, 1023,-24,24); 
        SquareV0 += sq(V0); 
        SquareV1 += sq(V1); 
        SquareV2 += sq(V2); 
        SquareV3 += sq(V3); 
        delayMicroseconds(16); // 16 us 1000 veces hacen un periodo de 0.16 s, o 62hz
    }

    SquareV0 = sqrt(SquareV0/SAMPLES); 
    SquareV1 = sqrt(SquareV1/SAMPLES); 
    SquareV2 = sqrt(SquareV2/SAMPLES); 
    SquareV3 = sqrt(SquareV3/SAMPLES);

    pantalla.setCursor(0, 1);
    imprimir_lcd("V0 RS=", SquareV0);

    pantalla.setCursor(0, 2);
    imprimir_lcd("V1 RMS=", SquareV1);   

    pantalla.setCursor(0, 3);
    imprimir_lcd("V2 RMS=", SquareV2);

    pantalla.setCursor(0, 4);
    imprimir_lcd("V3 RMS=", SquareV3);

    puerto_serial(SquareV0, SquareV1, SquareV2, SquareV3);
  }

  
}

void imprimir_lcd(char str[15], double V){
  pantalla.print(str); pantalla.print(V);
  }

void puerto_serial(double V0, double V1, double V2, double V3){
  Serial.print(V0); Serial.print(", ");Serial.print(V1); Serial.print(", ");Serial.print(V2); Serial.print(", ");Serial.println(V3);
  }