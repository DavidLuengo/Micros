/*
 * Bolera.c
 *
 * Created: 23/04/2018 15:41:15
 * Author : EquipoK
 */ 


//OJO PINES IBRA SI LOS HA CAMBIADO!!!

// Puerto J1:
//	Puerto B: motores 1, 3, 4, 5 (bajo: enable, alto: dir)
//	Puerto L: motor 2 (enable 0, dir 1, bk 2)
//	Puerto K: en k1 está el LED1, interrups

//	LIBRERIAS
#include <avr/io.h>
#include <avr/interrupt.h>

//          MACROS
#define setBit(P,B)    (P |= (0b00000001 << B))
#define clearBit(P,B)  (P &= (0b11111110 << B))  //hace falta función así el clearBit no va, change bit en función

#define OVERFLOWS_100_MS 13       //Timer 2 8bits,8Mhz, para 0.1 seg parpadeo LED son 13 veces desborde timer aprox
#define OVERFLOWS_11000_MS 1343   //Timer 1 16bits,8Mhz, elevar SW5 tras lanzamiento SW4 soltar, X veces desborde timer aprox

// Constante del delay
 #define DELAY 56


//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
	//estas son para los campos de la estructura motor, no para los puertos
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
	uint8_t bk;	//solo motor 2
	uint8_t index; // Indica el bit que usa para su señal de enable, el más bajo del grupo
	int retardo; // Rellenar este campo con el retardo especifico de cada motor
} motor;


//	VARIABLES
motor motor1 = {&PORTB, 0, 0, 0, 0, 10000};
motor motor2 = {&PORTL, 0, 0, 0, 0, 10000};
motor motor3 = {&PORTB, 0, 0, 0, 2, 10000};
motor motor4 = {&PORTB, 0, 0, 0, 4, 10000};
motor motor5 = {&PORTB, 0, 0, 0, 6, 10000};

	//para parpadeo LED cada 0.1s
uint8_t *P_extra = 0x00;  //valor de a lo que apunta puntero a 0, el puntero es para que se pueda acceder a la vble 
			 //desde cualquier función, si es vble solo en esa función
uint8_t overflowT2 = OVERFLOWS_100_MS; 

	// Vector de direcciones a los motores
motor *motores[] = {&motor1, &motor2, &motor3, &motor4, &motor5};

	// Variable de oscilacion. Vuelve a 0 cuando se pulsa SW6 para disparar
uint8_t *swing = 0x00;

	// Tiempo de retardo
 int retardo=0;
	

//	FUNCIONES

void setup(void){
	// FUNCION setBit    //LED apagado al comienzo, entiendo que es activo por nivel alto, esto pone a 0 (apaga)
				   //FALTA:cuando esté listo para lanzar poner setBit(K,1) PARA ENCENDER LED
	
	
	//Todos los timers Func normal y por overflow
	//El 0 para;el 2 para; el 1 para; el 3 para;el 4 para; el 5 para;
	cli();	
	
	TCCR0A = 0x00; 
	TCCR0B = 0x01; //sin prescalado 001;  //ver si hace falta al mirar tiempos, ver si está a 8MHZ
	TIMSK2 = 0x01;
	
	TCCR2A = 0x00; 
	TCCR2B = 0x01; //sin prescalado 001		//ver num overflows JulioJuan
	TIMSK2 = 0x01; 

	//Timer 1 CrisIbra lanz-elev; timer3 para swing para 1s? de swing centro-izq o izq-centro; 
	TCCR1A = 0x00; 					//falta mirar overflows a 8Mhz
	TCCR1B = 0x01; //sin prescalado 001
	TIMSK1 = 0x01; 
	
	sei();
	
	//Pone en marcha todos los motores hacia su posición de inicio.
	//Esperar un tiempo
	//Cuando todos se encuentren en su posicion original se carga la primera bola
	
	DDRB=0xff; //todos salidas motor 1, motor 3, motor 4, motor 5 enable y dirección
	DDRK=0b00000010; //PK1 salida Led, el resto entrada interrupciones sensores opticos
	DDRL=0b00000111; //PL0,PL1, PL2 salidas para el motor 2
	DDRD=0xff; //todos salida para el display
	
}

void swing(){
	cli(); //IMPORTANTE tiene sentido?
	PCMSK2 = 0x01; //Hemos activado Interrupción PCINT16 del PORTK
	PCICR= 0b00000100;//habilitadas interrupciones grupo 2 (de la 16 a la 23)
	//PCIFR=0b00000100;// habilito la bandera 
	//Hemos habilitado el grupo de interrupciones del PCINT16 al PCINT23
	sei();//habilitar interrupciones globales
	
	// Encender LED
	//setBit(PORTK,1);
	
	moveMotor(&motor2,DCHA);
	delay(3000);
	stopMotor(&motor2);
	delay(50);
	moveMotor(&motor2,IZDA);
	delay(3000);
	stopMotor(&motor2);
	delay(50);
}

void cargarbola(){
	if(PINL & 0b00100000){
		retardo=moveMotor(&motor2, IZDA)*0.5; //Lejos de recibir bola=IZDA
		delay(retardo);
	}
	if(PINL & 0b00000100){
		retardo=moveMotor(&motor1, DCHA);
		delay(10000);
	}
		retardo=cb1();
		delay(10000);
		
		retardo=cb2();
		delay(10000);
		
		retardo=cb3();
		delay(10000);
		
		retardo=cb4();
		delay(10000);
		
		retardo=cb5();
		delay(6000); //Aprox para poner en la mitad
		stopMotor(&motor2);
	
		swi = 1;
}

void inicializacion(){
	moveMotor(&motor5, IZDA); //Bajar=IZDA
	moveMotor(&motor4, IZDA); //Abrir=IZDA
	moveMotor(&motor1,DCHA);
	delay(10000);
	moveMotor(&motor2,DCHA);
	delay(10000);
	moveMotor(&motor1, IZDA); //recibo bola=DCHA
	delay(motor5.retardo); //Quiero tiempo suficiente para bajar motor5 y cargar bola en elevador de carga
	cargarbola();
}


void delay(int ms){
	for(int j = 0; j < ms; j++){
		for(i = 0; i < DELAY;){
			i++;
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
		 
		 } else if (M->port == &PORTB){
		 M->enable = OFF;
		 
		 mask = mask<<(M->index);
		 mask = ~mask;
		 
		 //aux = (direccion*2 + enable)*2^(M->index);
		 aux = (dir<<1)|enable;
		 aux = aux<<(M->index);
		 
		PORTB &= mask; // Deja libres los bits
		PORTB |= aux;
	 }
 }



void swing(){
	cli(); //IMPORTANTE tiene sentido?
	PCMSK2 = 0x01; //Hemos activado Interrupción PCINT16 del PORTK
	PCICR= 0b00000100;//habilitadas interrupciones grupo 2 (de la 16 a la 23)
	//PCIFR=0b00000100;// habilito la bandera 
	//Hemos habilitado el grupo de interrupciones del PCINT16 al PCINT23
	sei();//habilitar interrupciones globales
	
	// Encender LED
	//setBit(PORTK,1);
	
	moveMotor(&motor2,DCHA);
	delay(3000);
	stopMotor(&motor2);
	delay(50);
	moveMotor(&motor2,IZDA);
	delay(3000);
	stopMotor(&motor2);
	delay(50);
}

void cargarbola(){
	if(PINL & 0b00100000){
		retardo=moveMotor(&motor2, IZDA)*0.5; //Lejos de recibir bola=IZDA
		delay(retardo);
	}
	if(PINL & 0b00000100){
		retardo=moveMotor(&motor1, DCHA);
		delay(10000);
	}
		retardo=cb1();
		delay(10000);
		
		retardo=cb2();
		delay(10000);
		
		retardo=cb3();
		delay(10000);
		
		retardo=cb4();
		delay(10000);
		
		retardo=cb5();
		delay(6000); //Aprox para poner en la mitad
		stopMotor(&motor2);
	
		swi = 1;
}

void inicializacion(){
	moveMotor(&motor5, IZDA); //Bajar=IZDA
	moveMotor(&motor4, IZDA); //Abrir=IZDA
	moveMotor(&motor1,DCHA);
	delay(10000);
	moveMotor(&motor2,DCHA);
	delay(10000);
	moveMotor(&motor1, IZDA); //recibo bola=DCHA
	delay(motor5.retardo); //Quiero tiempo suficiente para bajar motor5 y cargar bola en elevador de carga
	cargarbola();
}

	
//funciones de movimiento de los motores para cada parte	
int cb1(){//3.5seg
	moveMotor(&motor2, DCHA); //Quiero ponerlo listo para recibir bola
	moveMotor(&motor3, DCHA); //avanza adelante=DCHA
	return motor3.retardo;
}

int cb2(){//1seg
	moveMotor(&motor2, IZDA);
	delay(120);
	dynamicstop(&motor2);
	moveMotor(&motor4, DCHA);//cierro compuerta=DCHA
	return motor4.retardo;
}

int cb3(){//3.5seg
	moveMotor(&motor3, IZDA);
	moveMotor(&motor1,IZDA);
	return 560; 
}

int cb4(){//1.5seg
	moveMotor(&motor1, DCHA); //Dejo pongo la bola lista en el lanzador
	return motor1.retardo;
}

int cb5(){//1seg
	//moveMotor(&motor1, IZDA);
	//delay(100);
	//dynamicstop(&motor1);
	moveMotor(&motor2, IZDA); //IMPORTANTE no se puede mover el motor 1 y motor 2 al mismo tiempo
	return 250;
}	
	

//	INTERRUPCIONES_Rutinas

// TIMERS, parece que no hace falta antirrebotes para el SW6
      
ISR(TIMER2_OVF_vect){				//Int period timer2  para 0.1s para parpadeo en p.extra una vez lanzada bola
	if (P_extra_lanz==1){   //vble que me diga que esta habilitado para lanzar en partida extra, definida pero no hecho código
		--overflowT2;									
		if(overflowT2 == 0){
			changeBit(&PORTK,1);
			overflowT2 = OVERFLOWS_100_MS ;  
		}
	}
}

//EXTERNAS
// SW6 pulsador
ISR(PCINT2_vect) {
	PCMSK2 = 0x00;	
	stopMotor(&motor2);
	moveMotor(&motor4, IZDA);//abre
	delay(motor4.retardo);
	TIMSK1 = 0x01; //Habilito la interrupción 13.5sec por overflow
	TCCR1B = 0x01;//Habilito la interrupción temporal con preescalado clk/1 de 16bits
	
	clearBit(&PORTK,1); // Apaga el LED
}




int overflowssw6 = OVERFLOWS_11000_MS;

ISR(TIMER1_OVF_vect){
	if(overflowssw6--==0){
		moveMotor(&motor5,DCHA);
		cargarbola();
		moveMotor(&motor5,IZDA);
	}
		
	/*for(overflowssw6=OVERFLOWS_11000_MS; overflowssw6>=0; overflowssw6--){
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
	*/
	
	TCCR1B =0x00;//Deshabilito la interrupcion temporal
}
	
//El timer 3 está libre, se puede usar para antirrebotes del SW6 pulsador
	
//	PROGRAMA PRINCIPAL
int main(void){
	setup();
	inicializacion();
    while (swi == 1) 
    {
		// Bucle principal
		swing();
    }
	
}
