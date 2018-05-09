/*
 * Bolera.c
 *
 * Created: 23/04/2018 15:41:15
 * Author : EquipoK
 */ 

// Puerto J1:
//	Puerto B: motores 1, 3, 4, 5 (bajo: enable, alto: dir)
//	Puerto L: motor 2 (enable 0, dir 1, bk 2)
//	Puerto K: en k1 está el LED1, interrups

//	LIBRERIAS
#include <avr/io.h>
#include <avr/interrupt.h>

//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
const uint8_t DCHA = 0x01;		// DERECHA
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

	// Tiempo de retardo
 int retardo=0;
	
	// Constante del delay
 #define DELAY 56

//	FUNCIONES


void delay(int ms){
	for(int j = 0; j < ms; j++){
		for(i = 0; i < DELAY;){
			i++;
		}
	}
}


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

int moveMotor(motor* M, uint8_t direccion){
	// Esta funcion mueve en la direccion asignada un motor
	// Ejemplo:	moveMotor(motores[1],0);
	//		moveMotor(&motor1,0);
	
	uint8_t aux = 0;
	uint8_t mask = 0b00000011;

	
	if (M->port == &PORTL){ //PUEDE QUE SEA NECESARIO AÑADIR EN EL IF LA COND DE BREAK=DEACT
		M->enable = ON;
		M->bk = DEACT;	// DEJAR LA SITUACIÓN DONDE BREAK=ACTIVO, PARA FUNCION DINAMICSTOP
		M->dir = direccion;
		
		aux = (M->bk<<2)|(M->dir<<1)|M->enable;
		
		mask = (mask<<1)|0x01;
		mask = ~mask;
		
		PORTL &= mask;
		PORTL |= aux;
		
	} else if (M->port == &PORTB){
		M->dir = direccion;
		M->enable = ON;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		//aux = (direccion*2 + enable)*2^(M->index);
		aux = (direccion<<1)|M->enable;
		aux = aux<<(M->index);
		
		PORTB &= mask; // Deja libres los bits
		PORTB |= aux;
		
		if(M->index==0){
			retardo=187; //1,5seg motor1
		} else if(M->index==2){
			retardo=437; //3,5seg motor3
		} else if(M->index==4){
			retardo=125; //1seg motor4
		} else {
			retardo=500; //4seg motor5
		}
	} 
	return retardo;
}

void dynamicstop(motor *M){//NOTA IMPORTANTE
	 
	uint8_t aux = 0;
	uint8_t mask = 0b00000011;
	uint8_t pivot=0x00;
	 
	uint8_t _enable = OFF;
	uint8_t _dir = M->dir;
	uint8_t _bk = DEACT;
	 
	if (M->port == &PORTL){ //PUEDE QUE SEA NECESARIO AÑADIR EN EL IF LA COND DE BREAK=DEACT
		 M->enable = OFF;
		 M->bk = ACT;	// DEJAR LA SITUACIÓN DONDE BREAK=ACTIVO, PARA FUNCION DINAMICSTOP
		 _bk = ACT;
		 
		 aux = (_bk<<2)|(_dir<<1)|_enable; //IMPORTANTE esto probablemente de problemas dado que bk, dir y enable se refieren a variables globales y no del struct
		 
		 mask = (mask<<1)|0x01;
		 mask = ~mask;
		 
		 pivot = PORTL & mask;
		 PORTL = pivot | aux;
		 M->dir=~M->dir;
		 
		 } else if (M->port == &PORTB){
		 M->enable = OFF;
		 
		 mask = mask<<(M->index);
		 mask = ~mask;
		 
		 //aux = (direccion*2 + enable)*2^(M->index);
		 aux = (dir<<1)|enable;
		 aux = aux<<(M->index);
		 
		PORTB &= mask; // Deja libres los bits
		PORTB |= aux;
		M->dir=~M->dir;
	 }
 }

//	FUNCIONES
void setup(void){
	//Pone en marcha todos los motores hacia su posición de inicio.
	//Esperar un tiempo
	//Cuando todos se encuentren en su posicion original se carga la primera bola
	
	DDRB=0xff; //todos salidas motor 1, motor 3, motor 4, motor 5 enable y dirección
	DDRK=0b00000010; //PK1 salida Led, el resto entrada interrupciones sensores opticos
	DDRL=0b00001011; //PL0,PL1, PL3 salidas para el motor 2
	DDRD=0xff; //todos salida para el display
	
	//Esperamos un tiempo a que todos los SW estén pulsados.
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

ISR(PCINT2_vect) {
	PCMSK2 = 0x00;	
	dynamicstop(&motor2);
	retardo=moveMotor(&motor4, IZDA);//abre
	delay(retardo);
	TIMSK1 = 0x01; //Habilito la interrupción 13.5sec por overflow
	TCCR1B = 0x01;//Habilito la interrupción temporal con preescalado clk/1 de 16bits
	
}

int cb1(){//3.5seg
	moveMotor(&motor2, DCHA); //Quiero ponerlo listo para recibir bola
	retardo=moveMotor(&motor3, DCHA); //avanza adelante=DCHA
	return retardo;
}

int cb2(){//1seg
	moveMotor(&motor2, IZDA);
	delay(120);
	dynamicstop(&motor2);
	retardo=moveMotor(&motor4, DCHA);//cierro compuerta=DCHA
	return retardo;
}

int cb3(){//3.5seg
	retardo=560;
	moveMotor(&motor3, IZDA);
	moveMotor(&motor1,IZDA);
	return retardo; 
}

int cb4(){//1.5seg
	retardo=moveMotor(&motor1, DCHA); //Dejo pongo la bola lista en el lanzador
	return retardo;
}

int cb5(){//1seg
	moveMotor(&motor1, IZDA);
	delay(100);
	dynamicstop(&motor1);
	moveMotor(&motor2, IZDA); //IMPORTANTE no se puede mover el motor 1 y motor 2 al mismo tiempo
	retardo=250;
	return retardo;
}

#define OVERFLOWS_11000_MS 1375
int overflowssw6 = OVERFLOWS_11000_MS;

ISR(TIMER1_OVF_vect){
	for(overflowssw6=OVERFLOWS_11000_MS; overflowssw6>=0; overflowssw6--){
		if(overflowssw6==1370){
			cb1();
		}
		if(overflowssw6==938){
			cb2();
		}
		if(overflowssw6==813){
			moveMotor(motores[5], motores[5]->dir);
			cb3();
		}
		if(overflowssw6==376){
			cb4();
		}
		if(overflowssw6==188){
			motores[5]->dir=!motores[5]->dir;
			moveMotor(motores[5], motores[5]->dir);
			cb5();
		}
	}
	TCCR1B =0x00;//Deshabilito la interrupcion temporal
}

void swing(){
	cli(); //IMPORTANTE tiene sentido?
	PCMSK2 = 0x01; //Hemos activado Interrupción PCINT16 del PORTK
	PCICR= 0b00000100; //Hemos habilitado el grupo de interrupciones del PCINT16 al PCINT23
	sei();//habilitar interrupciones globales
	
	if(!motores[2]->dir){
		retardo=moveMotor(&motor2,DCHA);//Multiplicar por una constante que permita un buen barrido
		delay(retardo);
		PORTL= 0b00000000;
		delay(50);
	} else {	
		retardo=moveMotor(&motor2,IZDA);
		delay(retardo);
		PORTL= 0b00000000;
		delay(50);
	}
}

void cargarbola(){
	if(PINL & 0b00100000){
		retardo=moveMotor(&motor2, IZDA)*0.5; //Lejos de recibir bola=IZDA
		delay(retardo);
	}
	if(PINL & 0b00000100){
		retardo=moveMotor(&motor1, DCHA);
		delay(retardo);
	}
		retardo=cb1();
		delay(retardo);
		
		retardo=cb2();
		delay(retardo);
		
		retardo=cb3();
		delay(retardo);
		
		retardo=cb4();
		delay(retardo);
		
		retardo=cb5();
		delay(retardo); //Aprox para poner en la mitad
}

void inicializacion(){
	retardo=moveMotor(&motor5, IZDA); //Bajar=IZDA
	moveMotor(&motor4, IZDA); //Abrir=IZDA
	moveMotor(&motor1, DCHA); //recibo bola=DCHA
	delay(retardo); //Quiero tiempo suficiente para bajar motor5 y cargar bola en elevador de carga
	cargarbola();
}

//	PROGRAMA PRINCIPAL
int main(void){
	setup();
	inicializacion();
    while (1) 
    {
		// Bucle principal
		swing();
    }
	
}
