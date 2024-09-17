#include <avr/io.h>
#include <stdbool.h> // para usar bools

#define INICIAL_PARPADEOS 4
#define INICIAL_PERIODO 2000 // en milisegundos

void parpadear(int cantidad, int periodo);
void emitirLuces(int parpadeos);
typedef enum {
    IDLE,               // Estado inicial
    PATRON,             // Patrón
    ESPERA_ENTRADA      // Espera entrada
} Estado;

int parpadeos = INICIAL_PARPADEOS;
int periodo = INICIAL_PERIODO;
volatile bool BotonEncendido = false;
volatile bool completo = false;
volatile bool delay = false;
volatile bool esEntradaCorrecta = false;
int patron[] = {1,2,4,3,1,1,3,4,2,3} //leds que se encenderán
int contador = 0;

// Lógica de entradas

ISR(PCINT0_vect){ //subrutina de interrupción con el vector pcint0
    switch (estado) {
            case IDLE:
                if ((PINB7 | PINB6 | PINB5 | PINB4) & (~BotonEncendido)){
                    BotonEncendido = true;
                }
                break;

            case ESPERA_ENTRADA:
            if (contador < parpadeos)
            {
                if (patron[contador] == 4) {
                    if(PINB7) {
                        if (~PINB7) contador++;} //para que se accione en el flanco negativo
                    else if (PINB6 | PINB5 | PINB4) completo = true; // la lógica de estados reconoce completo para que se salga de este flujo
                }                    
                else if (patron[contador] == 3) {
                    if (PINB6) {
                        if (~PINB6) contador++;}
                    else if (PINB7 | PINB5 | PINB4) completo = true;
                }
                else if (patron[contador] == 2) {
                    if (PINB5) {
                        if (~PINB5) contador++;}
                    else if (PINB6 | PINB7 | PINB4) completo = true;
                }
                else if (patron[contador] == 0) {
                    if (PINB4) {
                        if (~PINB4) contador++;}
                    else if (PINB6 | PINB5 | PINB7) completo = true;
                }
            
            } else{
                completo = true;
                esEntradaCorrecta = true; 
            }
            
            default:
                // Manejar estado desconocido
                //estado = IDLE;
                break;
    }break;
}


ISR(TIMER0_OVF_vect)
{
    // Code for handling Timer 0 Overflow
}

int main(void)
{
 DDRB = 0x0F; //Configuracion del puerto como entrada o salida
 GIMSK |= (1 << PCIE0);
 PCMSK0 |= (1 << PCINT7);
 PCMSK0 |= (1 << PCINT6);
 PCMSK0 |= (1 << PCINT5);
 PCMSK0 |= (1 << PCINT4);
 sei();

  //Parpadear
  while (1) {
        switch (estado) {
            case IDLE:
            // Configurar parpadeos iniciales y periodo               
                // Verificar botón
                if (BotonEncendido) {
                    parpadear(parpadeos, periodo);
                    estado = PATRON;
                }
                break;

            case PATRON:
                // Emitir luces para el patrón
                emitirLuces(parpadeos);
                estado = ESPERA_ENTRADA;
                break;

            case ESPERA_ENTRADA: // Esperar entrada del usuario

                // Verificar entrada del usuario
                if (esEntradaCorrecta & completo) {
                    // Ajustar configuración si la entrada es correcta                    
                    periodo -= 200 ;
                    esEntradaCorrecta = false; // reseteamos para la siguiente iteración
                    parpadeos++ ;  //aumenta un parpadeo
                    contador = 0; // se reinicia

                    // Verificar periodo para decidir el siguiente estado
                    if (periodo == 0) {
                        parpadear(2, periodo); // Parpadeo parpadeos
                        // Volver al estado inicial
                        estado = IDLE;
                    } else {
                        // Si el periodo no es 0, volver al patrón
                        estado = PATRON;
                    }

                } else if (~esEntradaCorrecta & completo){
                    // Si la entrada no es correcta, volver a IDLE
                    parpadear(3, periodo); // Parpadeo parpadeos
                    // Volver al estado inicial
                    estado = IDLE;
                }
                break;

            default:
                // Manejar estado desconocido
                estado = IDLE;
                break;
        }
    }
}

// Implementaciones de funciones

void parpadear(int cantidad, int periodo) {
    // Implementación para parpadear luces LED cantidad de veces con el periodo dado
    
    
}


void emitirLuces(int parpadeos) {
    for (int i = 0; i < parpadeos; i++){
        if (patron[i] == 4) {
            PORTB3 = 0b1; 
            delay = true;
            if (delay == false) PORTB3 = 0b0;  

        } else if (patron[i] == 3) {
            PORTB2 = 0b1; 
            delay = true;
            if (delay == false) PORTB2 = 0b0;  

        } else if (patron[i] == 2) {
            PORTB1 = 0b1; 
            delay = true;
            if (delay == false) PORTB1 = 0b0; 
             
        } else if (patron[i] == 0) {
            PORTB0 = 0b1;
            delay = true;
            if (delay == false) PORTB0 = 0b0; 
        }
    }
}



