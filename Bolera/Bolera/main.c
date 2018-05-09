/*
 * Bolera.c
 *
 * Created: 23/04/2018 15:41:15
 * Author : EquipoK
 */ 

// Puerto J1:
//	Puerto B: motores 1, 3, 4, 5 (bajo: enable, alto: dir)
//	Puerto L: motor 2 (enable 0, dir 1, bk 2)
//	Puerto K: en k1 está el LED1

//	LIBRERIAS
#include <avr/io.h>
#include <avr/interrupt.h>

//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
const uint8_t DCHA = 0x01;		// DERECHA                           //está bien?? salvo enables
const uint8_t IZDA = 0x00;		// IZQUIERDA
const uint8_t ON = 0x01;		// ENABLE ON
const uint8_t OFF = 0x00;		// ENABLE OFF
const uint8_t ACT = 0x01;		// BK ACTIVO (solo motor2)
const uint8_t DEACT = 0x00;		// BK INACTIVO (solo motor2)


typedef struct{
	volatile uint8_t * port;
	uint8_t enable;
	uint8_t dir;
	uint8_t bk;
	uint8_t index; // Indica el bit que usa para su señal de enable, el más bajo del grupo
	//A lo mejor hay que añadir una variable de velocidad
} motor;

//	VARIABLES
motor motor1 = {&PORTB, 0, 0, 0, 0};
motor motor2 = {&PORTL, 0, 0, 0, 0};
motor motor3 = {&PORTB, 0, 0, 0, 2};
motor motor4 = {&PORTB, 0, 0, 0, 4};
motor motor5 = {&PORTB, 0, 0, 0, 6};

	// Vector de direcciones a los motores
motor *motores[] = {&motor1, &motor2, &motor3, &motor4, &motor5};

	// Variable de oscilacion. Vuelve a 0 cuando se pulsa SW6 para disparar
uint8_t *swing = 0x00;

//	FUNCIONES

/*
void delay(int ms){
	int ciclos = ms * 2000;
	for(int i = 0; i < ciclos; i++){
		asm("nop");
	}
}

*/

void changeBit(uint8_t * puerto, uint8_t bit){
	//sintaxis:	changeBit( &POTRB , 2 )
	uint8_t p_aux = 0x00;
	uint8_t mask = 0x01;
	
	mask = mask << bit;
	
	p_aux = ~(*puerto);
	p_aux &= mask;
	
	*puerto &= ~mask;
	
	*puerto |= p_aux;
}

void moveMotor(motor* M, uint8_t direccion){
	// Esta funcion mueve en la direccion asignada un motor
	// Ejemplo:	moveMotor(motores[1],0);
	//		moveMotor(&motor1,0);
	
	uint8_t aux = 0x00;
	uint8_t mask = 0b00000011;
	
	
	if (M->port == &PORTL){
		M->enable = ON;
		M->bk = DEACT;	// Esto puede no estar bien
		M->dir = direccion;
		
		aux = (M->bk<<2)|(M->dir<<1)|M->enable;
		
		mask = (mask<<1)|0x01;
		mask = ~mask;
		
		PORTL &= mask;
		PORTL |= aux;
		
	}
	else if (M->port == &PORTB){
		M->dir = direccion;
		M->enable = ON;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		aux = (direccion<<1)|M->enable;
		aux = aux<<(M->index);
		
		PORTB &= mask; // Deja libres los bits
		PORTB |= aux;
	}
}

void stopMotor(motor* M){
	uint8_t aux = 0x00;
	uint8_t mask = 0b00000011;
	
	
	if (M->port == &PORTL){
		M->enable = OFF;
		M->bk = ACT;
		
		aux = (M->bk<<2)|(M->dir<<1)|M->enable;
		
		mask = (mask<<1)|0x01;
		mask = ~mask;
		
		PORTL &= mask;
		PORTL |= aux;
		
	}
	else if (M->port == &PORTB){
		M->enable = OFF;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		aux = (direccion<<1)|M->enable;
		aux = aux<<(M->index);
		
		PORTB &= mask; // Deja libres los bits
		PORTB |= aux;
	}
}

/*
void swingDelay(){
	//Esta es una version de la funcion de oscilacion usando delay
	*swing = 0x01;
	while(*swing == 0x01){
		moveMotor(&motor2,!&motor2->dir);
		delay(2000);
		stopMotor(&motor2);
		delay(50);
	}
}
*/

void setup(void){
	
//LED	
	#define setBit(P,B)    (P |= (0b00000001 << B))
	#define clearBit(P,B)  (P &= (0b11111110 << B))
	//hacer define que cambie un bit   #define changeBit(P,B) (P )
	
	//DDRK = 0xFF;             //OJO en config del puerto K poner el bit 1 para tenerlo como salida a 1
	clearBit (PORTK,1);         //LED apagado al comienzo, entiendo que es activo por nivel alto, esto pone a 0 (apaga)
				   //FALTA:cuando esté listo para lanzar poner setBit(K,1) PARA ENCENDER LED
	
	
	//Int period timer2  para 0.1s para parpadeo en p.extra una vez lanzada bola; sin prescalado-Func normal-por overflow
	cli();									
	TCCR2A = 0x00; 
	TCCR2B = 0x01; //sin prescalado 001
	TIMSK2 = 0x01; 
	//Int period timer3 para 1s? de swing centro-izq o izq-centro; sin prescalado-Func normalpor overflow
	TCCR2A = 0x00; 					//FALTA: subrutina swing con num overflows calcular
	TCCR2B = 0x01; //sin prescalado 001
	TIMSK2 = 0x01; 
	
	sei();
	
	//Pone en marcha todos los motores hacia su posición de inicio.
	//Esperar un tiempo
	//Cuando todos se encuentren en su posicion original se carga la primera bola
}


void lanzamiento (){						
		//cuando esté listo para lanzar y pulse SW6 pasarle qué vble? 

		//motor2 activo brake para pararlo, enable 0 y DI da = no?
setBit (K,1); //enciendo bit
		//lanza, retira M4--M4 en otra direcc DI cambio
	
	
}


//	INTERRUPCIONES_Rutinas

//externas
ISR()

// Timers, incluir antirreb si aplica
//para parpadeo LED cada 0.1s
	uint8_t P_extra_lanz = 0; 
	#define OVERFLOWS_100_MS 13     //OJO!!!!! A 8Mhz, para 0.1 seg son 13 veces desvorde timer aprox
	uint8_t overflowT2 = OVERFLOWS_100_MS;       
	ISR(TIMER0_OVF_vect){
		if (P_extra_lanz==1){   //vble que me diga que esta habilitado para lanzar en partida extra, sin hacer
 			--overflowT2;									
			if(overflows == 0){
			    //hacer MACRO change bit-----tipo PORTC = ~PINC;
			overflowT2 = OVERFLOWS_100_MS ;  
			}
		}
	}


//	PROGRAMA PRINCIPAL
int main(void)
{	
	setup();
	
   	while (1) 
    	{
		// Bucle principal
    	}
}

