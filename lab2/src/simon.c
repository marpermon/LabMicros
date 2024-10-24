#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h> // para usar bools

#define INICIAL_PARPADEOS 4
#define INICIAL_PERIODO 2000 // en milisegundos
#define PERIODO_OTROS 500 // en milisegundos

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
volatile int contador = 0;
volatile int patron[] = {1,2,4,3,1,1,3,4,2,3,2,4,1}; //leds que se encenderán
volatile bool rebote = true;
volatile bool revisar = false;
volatile bool BotonEncendido = false;
volatile bool completo = false;
volatile bool esEntradaCorrecta = false;
volatile bool PB7_anterior = false, PB6_anterior = false, PB5_anterior = false, PB4_anterior = false; //flanco decreciente
volatile bool PB7_actual = false, PB6_actual = false, PB5_actual = false, PB4_actual = false;
// Lógica de entradas



ISR(PCINT_B_vect){ //subrutina de interrupción con el vector pcint0
        PB7_actual = (PINB & (1 << PB7)) != 0;  // Read the state of PB7
        PB6_actual = (PINB & (1 << PB6)) != 0;  // Read the state of PB6
        PB5_actual = (PINB & (1 << PB5)) != 0;  // Read the state of PB5
        PB4_actual = (PINB & (1 << PB4)) != 0;  // Read the state of PB4
    
        if ((estado == IDLE)&&(rebote == false)) {
            if ((!PB7_anterior && PB7_actual) || (!PB6_anterior && PB6_actual) || (!PB5_anterior && PB5_actual) || (!PB4_anterior && PB4_actual)){               
                BotonEncendido = true;
            }
        }    
        
        PB7_anterior = PB7_actual;
        PB6_anterior = PB6_actual;
        PB5_anterior = PB5_actual;
        PB4_anterior = PB4_actual;
        if (estado == ESPERA_ENTRADA) revisar = true;
    
}


ISR(TIMER0_OVF_vect)
{
    millis_counter++;
}

int main(void)
{
    DDRB = 0x0F; //Configuracion del puerto como entrada o salida
    GIMSK |= (1 << 5);     // Habilitar interrupciones de cambio de estado para pin0
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
            delay_ms_p(100);
            rebote = false;
            // Configurar parpadeos iniciales y periodo 
            parpadeos = INICIAL_PARPADEOS;
            periodo = INICIAL_PERIODO;              
                // Verificar botón
                if (BotonEncendido) {
                    parpadear(2, PERIODO_OTROS);
                    BotonEncendido = false;
                    estado = PATRON;
                }
                break;

            case PATRON:
                // Emitir luces para el patrón
                emitirLuces(parpadeos, periodo);
                estado = ESPERA_ENTRADA;
                break;

            case ESPERA_ENTRADA: // Esperar entrada del usuario
                if (contador < parpadeos){
                    while (revisar){
                        switch (patron[contador]){
                            case 4:
                                if (!PB7_actual) contador++; //para que se accione en el flanco negativo
                                else if (!PB6_actual | !PB5_actual | !PB4_actual) completo = true; // la lógica de estados reconoce completo para que se salga de este flujo
                                break;
                            case 3:
                                if (!PB6_actual) contador++;
                                else if (!PB7_actual | !PB5_actual | !PB4_actual) completo = true;
                                break;
                            case 2:
                                if (!PB5_actual) contador++;
                                else if (!PB6_actual | !PB7_actual | !PB4_actual) completo = true;
                                break;
                            case 1:
                                if (!PB4_actual) contador++;
                                else if (!PB6_actual | !PB5_actual | !PB7_actual) completo = true;
                                break;            
                            default:
                                break;
                            }
                            revisar = false;        
                        }                     
                } else{
                    completo = true;
                    esEntradaCorrecta = true; 
                }

                // Verificar entrada del usuario
                if (esEntradaCorrecta & completo) {
                    // Ajustar configuración si la entrada es correcta                    
                    periodo -= 200 ;
                    esEntradaCorrecta = false; // reseteamos para la siguiente iteración
                    completo = false;
                    parpadeos++ ;  //aumenta un parpadeo
                    contador = 0; // se reinicia

                    // Verificar periodo para decidir el siguiente estado
                    if (periodo == 0) {
                        parpadear(4, PERIODO_OTROS); // Parpadeo 
                        // Volver al estado inicial
                        estado = IDLE;
                    } else {
                        // Si el periodo no es 0, volver al patrón
                        estado = PATRON;
                    }

                } else if (!esEntradaCorrecta & completo){
                    // Si la entrada no es correcta, volver a IDLE
                    parpadear(3, PERIODO_OTROS); // Parpadeo parpadeos
                    // Volver al estado inicial
                    estado = IDLE;
                    esEntradaCorrecta = false; // reseteamos para la siguiente iteración
                    completo = false;
                    contador = 0; // se reinicia
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
    delay_ms_p(300);
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