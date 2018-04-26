/*
 * BOLERA SISTEMA DE RETORNO.c
 *
 * Created: 26/04/2018 10:03:21
 * Author : crist
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
int setup(){
	
	
	
}
int arriba5;//podemos usar el SW5
int esperam5;

void activarmotor5(){
	
	if (arriba5=='0'){
		en5='1'; // declarar en5 y dir5
		dir5='1';
		esperam5=='1';
	}
	
	if (arriba5=='1'){
		activarmotor1();
		en5='1';
		dir5='0';
		esperam5=='0';	
	}
	
	if(SW5=='1' && en5='1'){//declarar SW5
		en5='0';
	}
	
	
}

void activarmotor1(){
	
	if (arriba1=='0'){
		en1='1'; // declarar en1 y dir 1
		dir1='1';
		esperam1=='1';
	}
	
	if (arriba1=='1'){
		en1='1';
		dir1='0';
		esperam1=='0';
	}
	
	if(SW1=='1' && en1='1'){ //declarar SW1
		en1='0';
	}
}

	
		
		
int main(void){
	setup();
	
}



#define OVERFLOWS_4500_MS 563
#define OVERFLOWS_2000_MS 250
#define OVERFLOWS_1500_MS 188

int overflowssw6 = OVERFLOWS_4500_MS;
int overflowssub5 = OVERFLOWS_2000_MS;
int overflowssub1= OVERFLOWS_1500_MS;

ISR(TIMER1_OVF_vect){
	if (SW6=='1'){ //declarar SW6
	   if(overflowssw6 -- == 0){
		  activarmotor5();
		  overflowssw6 = OVERFLOWS_4500_MS ;
		} 
	}
	
	if(esperam5=='1'){
		if(overflowssub5-- == 0){
			esperam5='0';
			activarmotor5();
			arriba5= ~arriba5;
			overflowssub5 = OVERFLOWS_2000_MS;
		}
	}
	
	if(esperam1=='1'){
		if(overflowssub1-- == 0){
			esperam1='0';
			activarmotor5();
			arriba1= ~arriba1;
			overflowssub1 = OVERFLOWS_1500_MS;
		}
	}
			
			
}


int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

