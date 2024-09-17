#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h> // para usar bools

#define INICIAL_PARPADEOS 4
#define INICIAL_PERIODO 2000 // en milisegundos

void delay_ms_p(int milliseconds); //funcion propia
void parpadear(int cantidad, int periodo);
void emitirLuces(int parpadeos, int periodo);
typedef enum {
    IDLE,               // Estado inicial
    PATRON,             // Patrón
    ESPERA_ENTRADA      // Espera entrada
} Estado;

volatile Estado estado = IDLE;
volatile int millis_counter = 0;
volatile int parpadeos = INICIAL_PARPADEOS;
volatile int periodo = INICIAL_PERIODO;
volatile bool BotonEncendido = false;
volatile bool completo = false;
volatile bool esEntradaCorrecta = false;
volatile int patron[] = {1,2,4,3,1,1,3,4,2,3}; //leds que se encenderán
volatile int contador = 0;

// Lógica de entradas

ISR(PCINT0_vect){ //subrutina de interrupción con el vector pcint0
    switch (estado) {
        case IDLE:
            if (~BotonEncendido){
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
    millis_counter++;
}

int main(void)
{
 DDRB = 0x0F; //Configuracion del puerto como entrada o salida
GIMSK |= 0x20; // 0x20 is 00100000 in binary, which sets the PCIE0 bit
PCMSK0 |= 0xF0; // 0xF0 is 11110000 in binary, which sets bits 4 to 7
 // Configure Timer0 in normal mode (default mode)
TCCR0A = 0; // Normal mode
TCCR0B = (1 << CS01) | (1 << CS00); // Set prescaler to 64
// Enable Timer0 overflow interrupt
TIMSK0 = (1 << TOIE0);
sei();

  //Parpadear
  while (1) {
        switch (estado) {
            case IDLE:
            // Configurar parpadeos iniciales y periodo               
                // Verificar botón
                if (BotonEncendido) {
                    parpadear(2, 100);
                    estado = PATRON;
                }
                break;

            case PATRON:
                // Emitir luces para el patrón
                emitirLuces(parpadeos, periodo);
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
                        parpadear(4, 100); // Parpadeo 
                        // Volver al estado inicial
                        estado = IDLE;
                    } else {
                        // Si el periodo no es 0, volver al patrón
                        estado = PATRON;
                    }

                } else if (~esEntradaCorrecta & completo){
                    // Si la entrada no es correcta, volver a IDLE
                    parpadear(3, 100); // Parpadeo parpadeos
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
    for (int i = 0; i < cantidad; i++){   
       PORTB |= 0x0F; // Turn on all LEDs on PB0-PB3
        delay_ms_p(periodo);
        PORTB &= ~0x0F; // Turn off all LEDs on PB0-PB3
        delay_ms_p(periodo);
    }
}

void emitirLuces(int parpadeos, int periodo) {
    for (int i = 0; i < parpadeos; i++){
        if (patron[i] == 4) {
            PORTB3 = 0b1; 
            delay_ms_p(periodo);
            PORTB3 = 0b0;
            delay_ms_p(periodo);  
        } else if (patron[i] == 3) {
            PORTB2 = 0b1; 
            delay_ms_p(periodo); 
            PORTB2 = 0b0;
            delay_ms_p(periodo);  
        } else if (patron[i] == 2) {
            PORTB1 = 0b1; 
            delay_ms_p(periodo); 
            PORTB1 = 0b0;
            delay_ms_p(periodo);          
        } else if (patron[i] == 0) {
            PORTB0 = 0b1;
            delay_ms_p(periodo); 
            PORTB0 = 0b0;
            delay_ms_p(periodo); 
        }
    }
}


void delay_ms_p(int milliseconds) { //funcion propia
    int start = millis_counter;
    while ((millis_counter - start) < milliseconds) {
        
    }
}




