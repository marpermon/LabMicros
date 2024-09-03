#include <pic14/pic12f683.h>
typedef unsigned int word;
word __at 0x2007 __CONFIG = _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _BOREN_OFF & _IESO_OFF & _FCMEN_OFF;
#define TIME 1

unsigned char lfsr = 0xFF, bit;
unsigned char actual = 1, nrandom=1, state;
enum estado {generando, emitiendo};
unsigned char numbers[]={'A','A','A','A','A','A','A','A','A','A'};
 
void delay (unsigned int tiempo);
void pulse (unsigned int tiempo);
 
void main(void){

     TRISIO = 0b00001000; //Poner todos los pines como salidas
	GPIO = 0x00; //Poner pines en bajo
	
	
		while(1){
			pulse(TIME);
			if (nrandom < 99) nrandom = nrandom + 1;
			else nrandom = 0;
			if(GP3==1){
				if(GP1 == 0){//para que sólo suceda la primera vez 
					for (int j=0; j<10; j++){
						if(nrandom == numbers[j]){
							pulse(TIME);
							if (nrandom < 99) nrandom = nrandom + 1;
							else nrandom = 0;
							break;
						}
					}
					GP1 = 1;
					for (int j=0; j<10; j++){
						if(numbers[j]=='A'){
							numbers[j] = nrandom;
							break;	 
						}
					}
				}
				else 	GP1 = 1;
			}
			else GP1 = 0;
			
			if(numbers[9]!='A'){
				GP2 = 1;
				delay(20);
				GP2 = 0;
				for (int j=0; j<98; j++){
					pulse(TIME);
				}
				delay(200);
				GP2 = 1;
				delay(20);
				GP2 = 0; 
				for (int j=0; j<98; j++){
					pulse(TIME);
				}
				delay(200);
				GP2 = 1;
				delay(20);
				GP2 = 0; 
				for (int j=0; j<98; j++){
					pulse(TIME);
				}
				delay(200);
				GP2 = 1;
				delay(20);
				GP2 = 0;
				for (int j=0; j<10; j++){
					numbers[j]='A';
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
	  for(j=0;j<10;j++);//1275=1s?
}
