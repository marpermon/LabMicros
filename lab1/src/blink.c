#include <pic14/pic12f683.h>
typedef unsigned int word;
word __at 0x2007 __CONFIG = _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _BOREN_OFF & _IESO_OFF & _FCMEN_OFF;
#define TIME 0.5

unsigned char lfsr = 0xFF, bit;
unsigned char actual = 1, nrandom=1, state;
enum estado {generando, emitiendo};
unsigned char numbers[10];
 
void delay (unsigned int tiempo);
void pulse (unsigned int tiempo);
 
void main(void){

     TRISIO = 0b00001000; //Poner todos los pines como salidas
	GPIO = 0x00; //Poner pines en bajo
	OPTION_REG = 0x07;
	//WPU = 0b00010000;
	//NOT_GPPU =0;
	bit = ((lfsr >> 7) ^ (lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3)) & 1;
        lfsr = (lfsr << 1) | bit;

        // Scale LFSR value to the range 0 to 99
        nrandom = (lfsr * 100) >> 8;	
 
	while(1){	
		while(GP3==0){
			pulse(TIME);
			actual++;
			if (actual >= nrandom) {
				while(GP3==0){};
			}												
		}
	}	
		//generar es para emitir la variable sólo una vezdespués de apretar el botón
							
}
 
void pulse (unsigned int tiempo)
{
	GP0 = 0x00;
	delay(tiempo);
	GP0 = ~GP0;
	delay(tiempo);
}

void delay(unsigned int tiempo)
{
	unsigned int i;
	unsigned int j;

	for(i=0;i<tiempo;i++)
	  for(j=0;j<1275;j++);//1275=1s?
}
