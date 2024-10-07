#define ANALOG0 A0 // Pines analógicos
#define ANALOG1 A1
#define ANALOG2 A2
#define ANALOG3 A3
#define ACDC 8 //Pin digital
#define SAMPLES 150


#include <PCD8544.h>
double V0, V1, V2, V3;
double MaxV0 = 0.0, MaxV1 = 0.0, MaxV2 = 0.0, MaxV3 = 0.0;
double impr[4] = {0,0,0,0};

PCD8544 pantalla;

void imprimir_lcd(char str[15], double V); // funcion para imprimir en la pantalla
void puerto_serial(double V0, double V1, double V2, double V3); // funcion para imprimir en el puerto serial

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Puerto serial   
  analogReference(5);// Desingnamos la referencia como la tensión máxima recibida por el arduino, que equivaldrá a 1023
  pinMode(ACDC, INPUT); // Usamos el interruptor como input
  pantalla.begin();
}

void loop() {


switch (digitalRead(ACDC)) {
  case HIGH: // En DC
    V0 = ((analogRead(ANALOG0)*5.0/1023.0)-2.45)*(50.0/5.0);   // leer entrada analógica y transformarla al rango original
    V1 = ((analogRead(ANALOG1)*5.0/1023.0)-2.45)*(50.0/5.0); 
    V2 = ((analogRead(ANALOG2)*5.0/1023.0)-2.45)*(50.0/5.0); 
    V3 = ((analogRead(ANALOG3)*5.0/1023.0)-2.45)*(50.0/5.0); 
    
    pantalla.setCursor(0, 1);
    if (abs(V0) > 24) pantalla.print("Peligro! V0  "); // Si el valor leído es es mayor a 24, encendemos alarma
    else imprimir_lcd("V0 =", V0); // Usamos la raíz cuadrada del valor máximo

    pantalla.setCursor(0, 2);
    if (abs(V1) > 24) pantalla.print("Peligro! V1  "); 
    else imprimir_lcd("V1 =", V1);   

    pantalla.setCursor(0, 3);
    if (abs(V2) > 24) pantalla.print("Peligro! V2  "); 
    else imprimir_lcd("V2 =", V2);

    pantalla.setCursor(0, 4);
    if (abs(V3) > 24) pantalla.print("Peligro! V3  "); 
    else imprimir_lcd("V3 =", V3);

    impr[0] = (abs(V0) > 24) ? 999 : V0; // Si el abs del valor leído es es mayor a 24 , indicamos un numero alto
    impr[1] = (abs(V1) > 24) ? 999 : V1;
    impr[2] = (abs(V2) > 24) ? 999 : V2;
    impr[3] = (abs(V3) > 24) ? 999 : V3; 

    puerto_serial(impr[0], impr[1], impr[2], impr[3]);
    

  break;

  case LOW: //En AC
    for (int i = 0; i < SAMPLES; i++) { 
    /* Usamos 500 samples en un periodo para determinar el
    valor pico de la señal*/
        V0 = ((analogRead(ANALOG0)*5.0/1023.0)-2.45)*(50.0/5.0);   // leer entrada analógica y transformarla al rango original
        V1 = ((analogRead(ANALOG1)*5.0/1023.0)-2.45)*(50.0/5.0); 
        V2 = ((analogRead(ANALOG2)*5.0/1023.0)-2.45)*(50.0/5.0); 
        V3 = ((analogRead(ANALOG3)*5.0/1023.0)-2.45)*(50.0/5.0);

        if (abs(V0) > 24) {
          pantalla.setCursor(0, 1);
          pantalla.print("Peligro! V0  "); // Si el valor leído es es mayor a 24, encendemos alarma
          MaxV0 = 1413; // 1413 entre raiz de 2 es aproximadamente 999
        } 
        else MaxV0 = (V0 > MaxV0) ? V0 : MaxV0; // Si el valor leído es es máximo se le asigna a la variable
        
        if (abs(V1) > 24) {
          pantalla.setCursor(0, 2);
          pantalla.print("Peligro! V1  ");
          MaxV1 = 1413;
        } 
        else MaxV1 = (V1 > MaxV1) ? V1 : MaxV1; 
        
        if (abs(V2) > 24) {
          pantalla.setCursor(0, 3);
          pantalla.print("Peligro! V2  ");
          MaxV2 = 1413;
        } 
        else MaxV2 = (V2 > MaxV2) ? V2 : MaxV2; 
        
        if (abs(V3) > 24) {
          pantalla.setCursor(0, 4);
          pantalla.print("Peligro! V3  ");
          MaxV3 = 1413;
        } 
        else MaxV3 = (V3 > MaxV3) ? V3 : MaxV3; 


    }


    pantalla.setCursor(0, 1);
    imprimir_lcd("V0 RMS=", MaxV0/sqrt(2)); //Usamos la raíz cuadrada del valor máximo

    pantalla.setCursor(0, 2);
    imprimir_lcd("V1 RMS=", MaxV1/sqrt(2));   

    pantalla.setCursor(0, 3);
    imprimir_lcd("V2 RMS=", MaxV2/sqrt(2));

    pantalla.setCursor(0, 4);
    imprimir_lcd("V3 RMS=", MaxV3/sqrt(2));

    impr[0] = MaxV0/sqrt(2); // Si el valor leído es es máximo se le asigna a la variable
    impr[1] = MaxV1/sqrt(2);
    impr[2] = MaxV2/sqrt(2);
    impr[3] = MaxV3/sqrt(2); 

    puerto_serial(impr[0], impr[1], impr[2], impr[3]);
  
  break;
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
