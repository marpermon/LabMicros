#include <avr/io.h>

#define INICIAL_PARPADEOS 4
#define INICIAL_PERIODO 2000 // en milisegundos

void parpadear(int cantidad, int periodo);
bool esBotonEncendido();
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

ISR(PCINT0_vect){ //subrutina de interrupción con el vector pcint0




}


int main(void)
{
 DDRB = 0x0F; //Configuracion del puerto como entrada o salida
 GIMSK |= (1 << PCIE0)
 PCMSK0 |= (1 << PCINT7)
 PCMSK0 |= (1 << PCINT6)
 PCMSK0 |= (1 << PCINT5)
 PCMSK0 |= (1 << PCINT4)
 sei()

int parpadeos = INICIAL_PARPADEOS;
int periodo = INICIAL_PERIODO;
  //Parpadear
  while (1) {
        switch (estado) {
            case IDLE:
                parpadeos = INICIAL_PARPADEOS;
                periodo = INICIAL_PERIODO;
                // Configurar parpadeos iniciales y periodo
                parpadear(parpadeos, periodo);

                // Verificar botón
                if (esBotonEncendido()) {
                    parpadear(2, periodo);
                    estado = PATRON;
                }
                break;

            case PATRON:
                // Emitir luces para el patrón
                emitirLuces();
                estado = ESPERA_ENTRADA;
                break;

            case ESPERA_ENTRADA:
                // Esperar entrada del usuario
                esperarEntrada();

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

bool esBotonEncendido() {
    // Implementación para verificar si el botón está encendido
    return false; // Placeholder
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

