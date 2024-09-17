#include <avr/io.h>
#include <stdbool.h> // para usar bools

#define INICIAL_PARPADEOS 4
#define INICIAL_PERIODO 2000 // en milisegundos

void parpadear(int cantidad, int periodo);
bool esEntradaCorrecta();
void emitirLuces();
void esperarEntrada();
void ajustarConfiguracion();
bool esPeriodoCero();
typedef enum {
    IDLE,               // Estado inicial
    PATRON,             // Patrón
    ESPERA_ENTRADA      // Espera entrada
} Estado;

int parpadeos = INICIAL_PARPADEOS;
int periodo = INICIAL_PERIODO;
volatile bool esBotonEncendido = 0;
volatile bool empezamos = 0;
volatile bool delay = 0;
int patron[10] = {1,2,4,3,1,1,3,4,2,3} //leds que se encenderán
int entrada[10];

// Lógica de entradas

ISR(PCINT0_vect){ //subrutina de interrupción con el vector pcint0
    switch (estado) {
            case IDLE:
                if ((PINB7 | PINB6 | PINB5 | PINB4) & (~esBotonEncendido)){
                    esBotonEncendido = 1;
                }
                break;

            case PATRON:
                break;

            case ESPERA_ENTRADA:
               
                break;

            default:
                // Manejar estado desconocido
                //estado = IDLE;
                break;
    }
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
                if (esBotonEncendido) {
                    parpadear(parpadeos, periodo);
                    estado = PATRON;
                }
                break;

            case PATRON:
                // Emitir luces para el patrón
                emitirLuces();
                estado = ESPERA_ENTRADA;
                break;

            case ESPERA_ENTRADA: // Esperar entrada del usuario

                // Verificar entrada del usuario
                if (esEntradaCorrecta()) {
                    // Ajustar configuración si la entrada es correcta
                    ajustarConfiguracion();

                    // Verificar periodo para decidir el siguiente estado
                    if (esPeriodoCero()) {
                        parpadear(2, periodo); // Parpadeo final
                        // Volver al estado inicial
                        estado = IDLE;
                    }else {
                    // Si el periodo no es 0, volver al patrón
                    estado = PATRON;
                }


                } else {
                    // Si la entrada no es correcta, volver al patrón
                    parpadear(3, periodo); // Parpadeo final
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

bool esEntradaCorrecta() {
    // Implementación para verificar si la entrada del usuario es correcta
    return false; // Placeholder
}

void emitirLuces() {
    // Implementación para emitir luces según el patrón
}

void esperarEntrada() {
    // Implementación para esperar la entrada del usuario
}

void ajustarConfiguracion() {
    // Incrementar parpadeos y disminuir el periodo
    parpadeos++;
    periodo -= 200; // Disminuir el periodo en 200 milisegundos
}

bool esPeriodoCero() {
    // Verificar si el periodo es cero
    return periodo <= 0;
}

