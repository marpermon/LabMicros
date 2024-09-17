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
static bool PB7_anterior; //flanco decreciente
static bool PB6_anterior; //flanco decreciente
static bool PB5_anterior; //flanco decreciente
static bool PB4_anterior; //flanco decreciente

// Lógica de entradas

ISR(PCINT_B_vect){ //subrutina de interrupción con el vector pcint0
bool PB7_actual = (PINB & (1 << PB7));
bool PB6_actual = (PINB & (1 << PB6));
bool PB5_actual = (PINB & (1 << PB5));
bool PB4_actual = (PINB & (1 << PB4));
    switch (estado) {
        case IDLE:
            if (!BotonEncendido){
                BotonEncendido = true;
            }
            break;

        case ESPERA_ENTRADA:
        if (contador < parpadeos)
        {
            if (patron[contador] == 4) {
                if (!PB7_actual && PB7_anterior) contador++; //para que se accione en el flanco negativo
                else if (PINB6 | PINB5 | PINB4) completo = true; // la lógica de estados reconoce completo para que se salga de este flujo
            }                    
            else if (patron[contador] == 3) {
                if (!PB6_actual && PB6_anterior) contador++;
                else if (PINB7 | PINB5 | PINB4) completo = true;
            }
            else if (patron[contador] == 2) {
                if (!PB5_actual && PB5_anterior) contador++;
                else if (PINB6 | PINB7 | PINB4) completo = true;
            }
            else if (patron[contador] == 0) {
                if (!PB4_actual && PB4_anterior) contador++;
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
    }
    PB7_anterior = PB7_actual;
    PB6_anterior = PB6_actual;
    PB5_anterior = PB5_actual;
    PB4_anterior = PB4_actual;
}


ISR(TIMER0_OVF_vect)
{
    millis_counter++;
}

int main(void)
{
    DDRB = 0x0F; //Configuracion del puerto como entrada o salida
    GIMSK |= (1 << PCIE);     // Habilitar interrupciones de cambio de estado para todos los pines
    PCMSK |= (1 << PCINT7);   // Habilitar interrupción en el pin PB7
    PCMSK |= (1 << PCINT6);   // Habilitar interrupción en el pin PB6
    PCMSK |= (1 << PCINT5);   // Habilitar interrupción en el pin PB5
    PCMSK |= (1 << PCINT4);   // Habilitar interrupción en el pin PB4


    // Configure Timer0 in normal mode (default mode)
    TCCR0A = 0; // Normal mode
    TCCR0B = (1 << CS01) | (1 << CS00); // Set prescaler to 64
    // Enable Timer0 overflow interrupt
    TIMSK |= (1 << TOIE0);
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

                } else if (!esEntradaCorrecta & completo){
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
        PORTB &= !0x0F; // Turn off all LEDs on PB0-PB3
        delay_ms_p(periodo);
    }
}

void emitirLuces(int parpadeos, int periodo) {
    for (int i = 0; i < parpadeos; i++){
        if (patron[i] == 4) {
            PORTB |= (1 << PB3); // Enciende el LED en PB3
            delay_ms_p(periodo);
            PORTB &= ~(1 << PB3); // Apaga el LED en PB3
            delay_ms_p(periodo);  
        } else if (patron[i] == 3) {
            PORTB |= (1 << PB2); // Enciende el LED en PB2
            delay_ms_p(periodo);
            PORTB &= ~(1 << PB2); // Apaga el LED en PB2
            delay_ms_p(periodo);  
        } else if (patron[i] == 2) {
            PORTB |= (1 << PB1); // Enciende el LED en PB1
            delay_ms_p(periodo);
            PORTB &= ~(1 << PB1); // Apaga el LED en PB1
            delay_ms_p(periodo);          
        } else if (patron[i] == 1) {
            PORTB |= (1 << PB0); // Enciende el LED en PB0
            delay_ms_p(periodo);
            PORTB &= ~(1 << PB0); // Apaga el LED en PB0
            delay_ms_p(periodo); 
        }
    }
}


void delay_ms_p(int milliseconds) { //funcion propia
    int start = millis_counter;
    while ((millis_counter - start) < milliseconds) {
        
    }
}




