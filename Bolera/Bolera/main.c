/*
Grupo K
 */ 


// Conector MOLASE J1:
//	Puerto B: motor 2 (enable 0, dir 1, bk 2)
//	Puerto L: motores 1, 3, 4, 5 (bajo: enable, alto: dir); intercambiamos B y L porque necesitábamos un pin para interrupción
//	Puerto K: interrupts, k1 es el LED

//	LIBRERIAS
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

//          MACROS
#define setBit(P,B)    (P |= (0b00000001 << B))
//funciones para cambiar bit y para limpiar bit, esto último no va como puse en otra macro 

#define OVERFLOWS_100_MS 3125       //Timer 2 8bits,8Mhz, para 0.1 seg parpadeo LED son 13 veces desborde timer aprox
#define OVERFLOWS_11000_MS 1343   //Timer 1 16bits,8Mhz, elevar SW5 tras lanzamiento SW4 soltar, X veces desborde timer aprox
//poner defines para el resto de timers tenerlos aquí

// Constante del delay
#define DELAY 56

//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
const uint8_t DCHA = 0x01;		// DERECHA
const uint8_t IZDA = 0x00;		// IZQUIERDA
const uint8_t ON = 0x01;		// ENABLE ON
const uint8_t OFF = 0x00;		// ENABLE OFF
const uint8_t ACT = 0x01;		// BK ACTIVO (solo motor2)
const uint8_t DEACT = 0x00;		// BK INACTIVO (solo motor2)

#define setBit(P,B) (P |= (0b00000001 << B))

uint8_t enable;
uint8_t dir;
uint8_t bk;
int retardo=0;
int swi=0;

// Contadores del swing
unsigned int *cont_T0 = 0;	// Timer0
unsigned int *cont_T3 = 0;	// Timer3
unsigned int *cont_SW2 = 0;	// SW2

typedef struct{
	volatile uint8_t * port;
	uint8_t enable;
	uint8_t dir;
	uint8_t bk;
	uint8_t index;
	int retardo;
} motor;

//	VARIABLES
motor motor1 = {&PORTL, 0, 0, 0, 0, 1000};
motor motor2 = {&PORTB, 0, 0, 0, 0, 1000};
motor motor3 = {&PORTL, 0, 0, 0, 2, 2334};
motor motor4 = {&PORTL, 0, 0, 0, 4, 667};
motor motor5 = {&PORTL, 0, 0, 0, 6, 2667};

//para parpadeo LED cada 0.1s
uint8_t *P_extra = 0x00;  //valor de a lo que apunta puntero a 0, el puntero es para que se pueda acceder a la vble 
			 //desde cualquier función, si es vble solo en esa función sería local
uint8_t overflowT2 = OVERFLOWS_100_MS; 

	// Vector de direcciones a los motores
motor *motores[] = {&motor1, &motor2, &motor3, &motor4, &motor5};

#define DELAY 56


//	FUNCIONES
void setup(void){
	//Pone en marcha todos los motores hacia su posición de inicio.
	//Esperar un tiempo
	//Cuando todos se encuentren en su posicion original se carga la primera bola

	
	DDRB=0xff;			//todos salidas motor 1, motor 3, motor 4, motor 5 enable y dirección
	DDRK=0b00000010;	//PK1 salida Led, el resto entrada interrupciones sensores opticos
	DDRL=0b00000111;	//PL0,PL1, PL2 salidas para el motor 2
						//PL3 => SW1, PL4 => SW3, PL5 => SW2, PL6 => SW5, PL7 => SW4
	DDRD=0xff;			//todos salida para el display
	
	//LED apagado al comienzo, entiendo que es activo por nivel alto
	PORTK &=11111101;
	
	
	//Todos los timers Func normal y por overflow
	//El 0 para;el 2 para; el 1 para; el 3 para;el 4 para; el 5 para;
	cli();	
	
	TCCR0A = 0x00; 
	TCCR0B = 0x01; //sin prescalado 001;  //ver num overflows JulioJuan
	TIMSK2 = 0x01;
	
	//Timer 2 para parpadeo Led en partida extra cada 0.1s cuando listo para lanzar
	TCCR2A = 0x00; 
	TCCR2B = 0x01; //sin prescalado 001		
	TIMSK2 = 0x01; 

	//Timer 1 CrisIbra lanz-elev; timer3 para swing para 1s? de swing centro-izq o izq-centro; 
	TCCR1A = 0x00; 					//falta mirar overflows a 8Mhz
	TCCR1B = 0x01; //sin prescalado 001
	TIMSK1 = 0x01; 
	
	//Timer 3 DavidTito Antirrebotes para SW2 posic medio
	TCCR3A = 0x00; 					//falta mirar overflows a 8Mhz y hacer subrutina
	TCCR3B = 0x01; //sin prescalado 001
	TIMSK3 = 0x01; 
	
	sei();
	
	
	//Esperamos un tiempo a que todos los SW estén pulsados.
	
}


void delay(int ms){
	for(int j=0; j<ms;j++){
		for(volatile unsigned i = 0; i<DELAY;){
			i++;
			//printf("a");
		}
	}
}


void changeBit(uint8_t * puerto, uint8_t bit){				//cambia el valor de cierto bit del puerto que sea
	//sintaxis:	changeBit( &POTRB , 2 )
	uint8_t p_aux = 0x00;	// Trabaja con los valores que hay en el puerto
	uint8_t mask = 0x01;	// Mascara del bit a cambiar
	
	mask = mask << bit;
	
	p_aux = ~(*puerto);
	p_aux &= mask;
	
	*puerto &= ~mask;
	*puerto |= p_aux;
}

void clearBit(uint8_t * puerto, uint8_t bit){
	uint8_t mask = 0x01;
	
	mask = mask << bit;
	*puerto &= ~mask;
}

void moveMotor(motor* M, uint8_t direccion){
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
	} 
}

void stopMotor(motor *M){//NOTA IMPORTANTE
	
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
		
		//Cambio automatico de direccion
		//M->dir=~M->dir;
		
		} else if (M->port == &PORTB){
		M->enable = OFF;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		//aux = (direccion*2 + enable)*2^(M->index);
		aux = (dir<<1)|enable;
		aux = aux<<(M->index);
		
		PORTB &= mask; // Deja libres los bits
		PORTB |= aux;
		
		//Cambio automatico de direccion
		//M->dir=~M->dir;
	}
}

//	FUNCIONES


//	INTERRUPCIONES

// Timer 3
ISR(TIMER3_OVF_vect){
	if(*cont_T3 < 25){
		*cont_T3++;
	}
	
	else{
		cli();
		// Se deshabilita TIMER3
		TCCR3B = 0X00;
		// Se habilita SW2
		
		sei();
		// Reiniciar contador
	}
	
}

// Timer 0
ISR(TIMER0_OVF_vect){
	if(*cont_T0 < 2){
		*cont_T0++;
	}
	else{
		// Deshabilitar TIMER0
		cli();
		TCCRB0 = 0x00;
		sei();
		// Cambias direccion motor2
		moveMotor(&motor2,!motor2.dir);
		// Reiniciar contador
		*cont_T0 = 0;
	}
}

// Interrupcion SW6
ISR(PCINT2_vect) {
	PCMSK2 = 0x00;	
	stopMotor(&motor2);
	moveMotor(&motor4, IZDA);//abre
	//delay(3000);
	swi = 0;
	TIMSK1 = 0x01; //Habilito la interrupción 13.5sec por overflow
	TCCR1B = 0x01;//Habilito la interrupción temporal con preescalado clk/1 de 16bits
	
}

// Interruocion SW2
ISR(//poner el 	que sea){
	
	
	if(*cont_SW2 == 0x00){
		cli();
		//SW6
		PCMSK2 = 0x01; //Hemos activado Interrupción PCINT16 del PORTK
		PCICR= 0b00000100;//habilitadas interrupciones grupo 2 (de la 16 a la 23)
		sei();
		setBit(&PORTK,1);
		*cont_SW2 = 0x01;
	}
	
	else if(*cont_SW2 == 0x01){
		// Frenar el motor
		stopMotor(&motor2);
		
		cli();
		// Habilitar timer 0 (freno)
		TCCR0A = 0x00;
		TCCR0B = 0x05; //Prescalado 1024 => 101		CONTAMOS 2 OVERFLOWS en lugas de los 1.52 veces necesarias
		TIMSK0 = 0x01;
		
		// Habilitar timer 3 (vuelta)	=>	QUITAR DEL SETUP
		TCCR3A = 0x00;
		TCCR3B = 0x01;
		TIMSK3 = 0x01;
		// Deshabilitamos esta interrupcion
		
		
		sei();
		
		
	}
}

int cb1(){//3.5seg
	moveMotor(&motor2, DCHA); //Quiero ponerlo listo para recibir bola
	moveMotor(&motor3, DCHA); //avanza adelante=DCHA
	delay(15000);
	return motor3.retardo;
}

int cb2(){//1seg
	//moveMotor(&motor2, IZDA);
	//delay(130);
	//dynamicstop(&motor2);
	moveMotor(&motor4, DCHA);//cierro compuerta=DCHA
	delay(15000);
	return motor4.retardo;
}

int cb3(){//3.5seg
	moveMotor(&motor3, IZDA);
	moveMotor(&motor1,IZDA);
	delay(15000);
	return 560; 
}

int cb4(){//1.5seg
	moveMotor(&motor1, DCHA); //Dejo pongo la bola lista en el lanzador
	moveMotor(&motor2, IZDA);
	delay(130);
	stopMotor(&motor2);
	delay(15000);
	return motor1.retardo;
}

int cb5(){//1seg
//	moveMotor(&motor1, IZDA);
	//delay(100);
	//dynamicstop(&motor1);
	moveMotor(&motor2,IZDA); //IMPORTANTE no se puede mover el motor 1 y motor 2 al mismo tiempo
	return 10000;
}

//cargar bola
void cargarbola(){
	if(PINL & 0b00100000){
		moveMotor(&motor2, IZDA); //Lejos de recibir bola=IZDA
		delay(motor2.retardo >> 1); // motor2.retardo >> 1 equivale a motor2.retardo*0.5 
	}
	if(PINL & 0b00001000){//Verificar que cuando SW2 esta activo M1 se sube
		moveMotor(&motor1, DCHA);
		delay(motor1.retardo);
	}
	cb1();
	delay(10000);
	
	cb2();
	delay(10000);
	
	cb3();
	delay(10000);
	
	cb4();
	delay(10000);
	
	cb5();
	delay(6000); //Aprox para poner en la mitad
	stopMotor(&motor2);
	
	swi = 1;
}

#define OVERFLOWS_11000_MS 1375
int overflowssw6 = OVERFLOWS_11000_MS;

ISR(TIMER1_OVF_vect){
	
	if(overflowssw6--==0){
		moveMotor(&motor5,DCHA);
		cargarbola();
		moveMotor(&motor5,IZDA);}
		
	
	TCCR1B = 0x00;//Deshabilito la interrupcion temporal
	
}


void swing(){
	// Encender LED
	//setBit(PORTK,1);
	/*
	    moveMotor(&motor2,DCHA);
		delay(3000);
		stopMotor(&motor2);
		delay(50);
		moveMotor(&motor2,IZDA);
		delay(3000);
		stopMotor(&motor2);
		delay(50);
	*/
	
	// Funcionamiento:
	// 1) Avanza hacia la izda
	// 2) Toca SW2_medio => Cont++
	// 3) Toca SW2_izda => Cont++
	// 4) A partir de aqui, siempre que se detecte un SW2
	//	- Se deeshabilita SW2
	//	- Se para el motor y se espera un tiempo para el frenado
	//	- Se pone en marcha en otro sentido
	//	- Despues de un tiempo suficiente se habilita SW2
	// 5) Cuando se pulsa SW6, se reinician los valores de swing()

	// Cosas necesarias:
	// Variable que cuente
	// Contador 8 bits
	// Interrupcion y pin para SW2
	
	uint8_t cont = 0
	
	if(cont == 1){
		cli();
		//SW6
		PCMSK2 = 0x01; //Hemos activado Interrupción PCINT16 del PORTK
		PCICR= 0b00000100;//habilitadas interrupciones grupo 2 (de la 16 a la 23)
		//SW2
		
		sei();
		
		setBit(&PORTK,1);
	}
}



void inicializacion(){
	moveMotor(&motor5, IZDA); //Bajar=IZDA
	moveMotor(&motor4, IZDA); //Abrir=IZDA
	
	moveMotor(&motor1,DCHA);
	
	delay(300);
	moveMotor(&motor2,DCHA);
	delay(10000);
	moveMotor(&motor1, IZDA); //recibo bola=DCHA
	delay(motor5.retardo); //Quiero tiempo suficiente para bajar motor5 y cargar bola en elevador de carga
	cargarbola();
}

//	PROGRAMA PRINCIPAL

int main(void){
	
    setup();
	inicializacion();
	while(1){
		if(swi == 1){
			swing();
		}
	}
   
}
