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
 	unsigned int antiguo = 0;
	unsigned int actual = 0;
	unsigned int generar = 0;
	unsigned int nrandom = 0;
    	unsigned int time = 0.05;
	unsigned int numbers[10];
	unsigned int j = 0;
 
    //Loop forever
    while ( 1 )
    {
		antiguo = actual;
		actual = GP3;
		//GP4 = GP3;
		
		if ((antiguo==1)&&(actual==0)) generar = 1;
		
		while ((generar == 1)&&(GP3==0)){
			for(int i=0;i<nrandom;i++){
				GP0 = 0x00;
				delay(time);
				GP0 = ~GP0;
				delay(time);
			}
			generar=0;			
		}

		while (GP3 == 1){	//generar numero pseudorandom
			//nrandom = (nrandom == 99) ? 0 : nrandom + 1; //que no se pase de 99
			if (nrandom == 99) nrandom = 0;
			else nrandom++;
			}

		
		
		
		//generar es para emitir la variable sólo una vezdespués de apretar el botón
							
    }
 

}

void delay(unsigned int tiempo)
{
	unsigned int i;
	unsigned int j;

	for(i=0;i<tiempo;i++)
	  for(j=0;j<1275;j++);//1275=1s?
}
