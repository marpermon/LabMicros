#include <pic14/pic12f683.h>
typedef unsigned int word ;
word __at 0x2007 __CONFIG = _WDTE_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _BOREN_OFF & _IESO_OFF & _FCMEN_OFF;

 
void delay (unsigned int tiempo);
 
void main(void)
{

     TRISIO = 0b00001000; //Poner todos los pines como salidas
	GPIO = 0x00; //Poner pines en bajo
	//WPU = 0b00010000;
	//NOT_GPPU =0;

	unsigned short int actual = 0, nrandom = 0;
	enum estado {generando, emitiendo};
	unsigned short int state = generando;
    	float time = 0.1;
	unsigned int numbers[10];
	
 
while(1){
	
	switch (state){
		case generando:
		//la primera vez, si GP3==0 no se hace nada hasta que se active
			while (GP3 == 1){	//generar numero pseudorandom
				actual = 0;
				if (nrandom < 99) nrandom = nrandom + 1;
				else nrandom = 0; //que no se pase de 99
			}
			state = generando;
		
		case emitiendo:	
			while((GP3==0)&&(actual < nrandom))
				{
				GP0 = 0x00;
				delay(time);
				GP0 = ~GP0;
				delay(time);
				actual++;									
			}
			if (actual>=nrandom ) state = generando;
		}
	}	
		//generar es para emitir la variable sólo una vezdespués de apretar el botón							
}
 


void delay(unsigned int tiempo)
{
	unsigned int i;
	unsigned int j;

	for(i=0;i<tiempo;i++)
	  for(j=0;j<1275;j++);//1275=1s?
}
