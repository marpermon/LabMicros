#define ANALOG0 A0 // Pines analógicos
#define ANALOG1 A1
#define ANALOG2 A2
#define ANALOG3 A3
#define ACDC 8 //Pin digital
#define SAMPLES 500


#include <PCD8544.h>
double V0, V1, V2, V3;
double MaxV0 = 0.0, MaxV1 = 0.0, MaxV2 = 0.0, MaxV3 = 0.0;
double referencia = 4.84; // tensión máxima de las señales

PCD8544 pantalla;

void imprimir_lcd(char str[15], double V); // funcion para imprimir en la pantalla
void puerto_serial(double V0, double V1, double V2, double V3); // funcion para imprimir en el puerto serial

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Puerto serial   
  analogReference(referencia);// Desingnamos la referencia como la tensión máxima recibida por el arduino, que equivaldrá a 1023
  pinMode(ACDC, INPUT); // Usamos el interruptor como input
  pantalla.begin();
}

void loop() {


switch (digitalRead(ACDC)) {
  case HIGH: // En DC
    V0 = (analogRead(ANALOG0) * 48.0 / 1023.0) - 24.0;   // leer entrada analógica y transformarla al rango original
    V1 = (analogRead(ANALOG1) * 48.0 / 1023.0) - 24.0; 
    V2 = (analogRead(ANALOG2) * 48.0 / 1023.0) - 24.0; 
    V3 = (analogRead(ANALOG3) * 48.0 / 1023.0) - 24.0; 
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
    /* Usamos 500 samples en un periodo para determinar el
    valor pico de la señal*/
        V0 = (analogRead(ANALOG0) * 48.0 / 1023.0) - 24.0;   // leer entrada analógica y transformarla al rango original
        V1 = (analogRead(ANALOG1) * 48.0 / 1023.0) - 24.0; 
        V2 = (analogRead(ANALOG2) * 48.0 / 1023.0) - 24.0; 
        V3 = (analogRead(ANALOG3) * 48.0 / 1023.0) - 24.0;
        MaxV0 = (V0 > MaxV0) ? V0 : MaxV0; // Si el valor leído es es máximo se le asigna a la variable
        MaxV1 = (V1 > MaxV1) ? V1 : MaxV1;
        MaxV2 = (V2 > MaxV2) ? V2 : MaxV2;
        MaxV3 = (V3 > MaxV3) ? V3 : MaxV3; 
    }


    pantalla.setCursor(0, 1);
    imprimir_lcd("V0 RMS=", MaxV0/sqrt(2)); //Usamos la raíz cuadrada del valor máximo

    pantalla.setCursor(0, 2);
    imprimir_lcd("V1 RMS=", MaxV1/sqrt(2));   

    pantalla.setCursor(0, 3);
    imprimir_lcd("V2 RMS=", MaxV2/sqrt(2));

    pantalla.setCursor(0, 4);
    imprimir_lcd("V3 RMS=", MaxV3/sqrt(2));

    puerto_serial(MaxV0/sqrt(2), MaxV1/sqrt(2), MaxV2/sqrt(2), MaxV3/sqrt(2));
  }

  
}

void imprimir_lcd(char str[15], double V){ 
  pantalla.print(str); pantalla.print(V);
  }

void puerto_serial(double V0, double V1, double V2, double V3){
  Serial.println(V0); 
  Serial.println(V1);
  Serial.println(V2);
  Serial.println(V3);
  }
