/*
Grupo K
 */ 

int PrimeraTirada = 1,
bandera1= 0, bandera2= 0, bandera3= 0, bandera4= 0,bandera5= 0, bandera6= 0, contadorparp =0, a=0,
puntuacion= 0, un = 0, de= 0, cambio = 1, ultima= 0, ultimatirada = 0, fin= 0, derecha = 0, izquierda = 0;
int unidades=0, decenas =0;

int pulsado=0;


// Conector MOLASE J1:
//	Puerto B: motor 2 (enable 0, dir 1, bk 2)
//	Puerto L: motores 1, 3, 4, 5 (bajo: enable, alto: dir); intercambiamos B y L porque necesitábamos un pin para interrupción
//	Puerto K: interrupts, k1 es el LED

//	LIBRERIAS
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>


//          MACROS
//#define setBit(P,B) (P) |= (1<<B)
//funciones para cambiar bit y para limpiar bit, esto último no va como puse en otra macro 

#define OVERFLOWS_100_MS 3125       //Timer 2 8bits,8Mhz, para 0.1 seg parpadeo LED son 13 veces desborde timer aprox
#define OVERFLOWS_6000 750   //Timer 1 16bits,8Mhz, elevar SW5 tras lanzamiento SW4 soltar, X veces desborde timer aprox
#define SegDeJuego 3662
#define SegDeTemp 13
#define SegDeTemp2 563
//poner defines para el resto de timers tenerlos aquí
int overflowssw6 = 0;
int contador = 0;			
int contador01 = 0;
int contador02 = 0;


// Constante del delay: calculada empiricamente con simulacion
#define DELAY 420

//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
const uint8_t DCHA = 0x01;		// DERECHA
const uint8_t IZDA = 0x00;		// IZQUIERDA
const uint8_t ON = 0x01;		// ENABLE ON
const uint8_t OFF = 0x00;		// ENABLE OFF
const uint8_t ACT = 0x01;		// BK ACTIVO (solo motor2)
const uint8_t DEACT = 0x00;		// BK INACTIVO (solo motor2)


uint8_t enable;
uint8_t dir;
uint8_t bk;
int retardo=0;
//int swi=0;

// Contadores del swing
unsigned int cont_T0 = 0;	// Timer0
unsigned int cont_T2 = 0;	// Timer2
unsigned int cont_T3 = 0;	// Timer3
unsigned int cont_SW2 = 0;	// SW2

typedef struct{
	volatile uint8_t * port;
	uint8_t enable;
	uint8_t dir;
	uint8_t bk;
	uint8_t index;
	int retardo;
} motor;

//	VARIABLES
motor motor1 = {&PORTL, 0, 0, 0, 0, 134};
motor motor2 = {&PORTB, 0, 0, 0, 0, 134};
motor motor3 = {&PORTL, 0, 0, 0, 2, 311};
motor motor4 = {&PORTL, 0, 0, 0, 4, 89};
motor motor5 = {&PORTL, 0, 0, 0, 6, 356};

//para parpadeo LED cada 0.1s
uint8_t P_extra = 0x00;  //valor de a lo que apunta puntero a 0, el puntero es para que se pueda acceder a la vble 
			 //desde cualquier función, si es vble solo en esa función sería local
			 // NOTA: ver la nomenclatura en otros codigos

	// Vector de direcciones a los motores
motor *motores[] = {&motor1, &motor2, &motor3, &motor4, &motor5};


//	FUNCIONES
void setup(void){
	//Pone en marcha todos los motores hacia su posición de inicio.
	//Esperar un tiempo
	//Cuando todos se encuentren en su posicion original se carga la primera bola

	
	DDRL=0xFF;			//todos salidas motor 1, motor 3, motor 4, motor 5 enable y dirección
	DDRK=0b00000010;	//PK1 salida Led, el resto entrada interrupciones sensores opticos
	DDRB=0b00000111;	//PB0,PB1, PB2 salidas para el motor 2
						//PB3 => SW1, PB4 => SW3, PB5 => SW2, PB6 => SW5, PB7 => SW4
	DDRD=0xFF;			//todos salida para el display
	

	
	//LED apagado al comienzo, entiendo que es activo por nivel alto
	PORTK &=11111101;
	
	
	//Todos los timers Func normal y por overflow
	//El 0 para;el 2 para; el 1 para; el 3 para;el 4 para; el 5 para;
	cli();	
	
	TCCR0A = 0x00;
	TCCR0B = 0b00000011; // *Empieza a contar el temporizador del display q alterna unidades y decenas a 8192 microsegundos.
    TIMSK0 = 0x01; //
	
	//Timer 2 para parpadeo Led en partida extra cada 0.1s cuando listo para lanzar
	TCCR2A = 0x00; 
	TCCR2B = 0x00; // Deshabilitado		
	TIMSK2 = 0x01; 

	//Timer 1 CrisIbra lanz-elev; timer3 para swing para 1s? de swing centro-izq o izq-centro; 
	TCCR1A = 0x00; 					//falta mirar overflows a 8Mhz
	TCCR1B = 0x00; // Deshabilitado
	TIMSK1 = 0x01; 
	
	//Timer 3 DavidTito Antirrebotes para SW2 posic medio
	TCCR3A = 0x00; 					//falta mirar overflows a 8Mhz y hacer subrutina
	TCCR3B = 0x00; // Deshabilitado
	TIMSK3 = 0x01; 
	
	//habilitadas interrupciones grupo 2 (de la 16 a la 23) y el grupo 0 (de la 0 a la 7)
	PCICR = 0b00000101;
	PCMSK0 = 0x00;
	PCMSK2 = 0x00;
	
	sei();
	
	//Esperamos un tiempo a que todos los SW estén pulsados.
	
}




void pintar(int un, int de) {

    switch (un) {

    case 0 :  unidades = 0b00111111; break;
	case 1 :  unidades = 0b00000110; break;
    case 2 :  unidades = 0b01011011; break;
    case 3 :  unidades = 0b01001111; break;
    case 4 :  unidades = 0b01100110; break;
    case 5 :  unidades = 0b01101101; break;
    case 6 :  unidades = 0b01111101; break;
    case 7 :  unidades = 0b00000111; break;
    case 8 :  unidades = 0b01111111; break;
    case 9 :  unidades = 0b01101111; break;
	default:  unidades = 0b00000000;  //*display apagado durante el parpadeo


}
    switch (de) {

    case 0 :  decenas = 0b10111111; break;
    case 1 :  decenas = 0b10000110; break;
    case 2 :  decenas = 0b11011011; break;
    case 3 :  decenas = 0b11001111; break;
    case 4 :  decenas = 0b11100110; break;
    case 5 :  decenas = 0b11101101; break;
    case 6 :  decenas = 0b11111101; break;
    case 7 :  decenas = 0b10000111; break;
    case 8 :  decenas = 0b11111111; break;
    case 9 :  decenas = 0b11101111; break;
	default:  decenas = 0b10000000; //*display apagado durante el parpadeo
	
}
}



void delay(int ms){
	for(int j=0; j<ms;j++){
		for(volatile unsigned i = 0; i < DELAY; i++){
			asm("nop");
		}
	}
}


void changeBit(volatile uint8_t * puerto, uint8_t bit){	//cambia el valor de cierto bit del puerto que sea
	//sintaxis:	changeBit( &POTRB , 2 )
	uint8_t p_aux = 0x00;	// Trabaja con los valores que hay en el puerto
	uint8_t mask = 0x01;	// Mascara del bit a cambiar
	
	mask = mask << bit;
	
	p_aux = ~(*puerto);
	p_aux &= mask;
	
	*puerto &= ~mask;
	*puerto |= p_aux;
}

void clearBit(volatile uint8_t * puerto, uint8_t bit){	
	uint8_t mask = 0x01;
	
	mask = mask << bit;
	*puerto &= ~mask;
}

void setBit(volatile uint8_t * puerto, uint8_t bit){
	uint8_t mask = 0x01;
	
	mask = mask << bit;
	*puerto |= mask;
}

void moveMotor(motor* M, uint8_t direccion){
	// Esta funcion mueve en la direccion asignada un motor
	// Ejemplo:	moveMotor(motores[1],0);
	//		moveMotor(&motor1,0);
	
	uint8_t aux = 0;
	uint8_t mask = 0b00000011;

	
	if (M->port == &PORTB){ //PUEDE QUE SEA NECESARIO AÑADIR EN EL IF LA COND DE BREAK=DEACT
		M->enable = ON;
		M->bk = DEACT;	// DEJAR LA SITUACIÓN DONDE BREAK=ACTIVO, PARA FUNCION DINAMICSTOP
		M->dir = direccion;
		
		aux = (M->bk<<2)|(M->dir<<1)|M->enable;
		
		mask = (mask<<1)|0x01;
		mask = ~mask;
		
		PORTB &= mask;
		PORTB |= aux;
		
	} else if (M->port == &PORTL){
		M->dir = direccion;
		M->enable = ON;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		aux = (direccion<<1)|M->enable;
		aux = aux<<(M->index);
		
		PORTL &= mask; // Deja libres los bits
		PORTL |= aux;
	} 
}

void stopMotor(motor *M){//NOTA IMPORTANTE
	
	uint8_t aux = 0;
	uint8_t mask = 0b00000011;
	uint8_t pivot=0x00;
	
	uint8_t _enable = OFF;
	uint8_t _dir = M->dir;
	uint8_t _bk = DEACT;
	
	if (M->port == &PORTB){ //PUEDE QUE SEA NECESARIO AÑADIR EN EL IF LA COND DE BREAK=DEACT
		M->enable = OFF;
		M->bk = ACT;	// DEJAR LA SITUACIÓN DONDE BREAK=ACTIVO, PARA FUNCION DINAMICSTOP
		_bk = ACT;
		
		aux = (_bk<<2)|(_dir<<1)|_enable; //IMPORTANTE esto probablemente de problemas dado que bk, dir y enable se refieren a variables globales y no del struct
		
		mask = (mask<<1)|0x01;
		mask = ~mask;
		
		pivot = PORTB & mask;
		PORTB = pivot | aux;
		}
		else if (M->port == &PORTL){
		M->enable = OFF;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		//aux = (direccion*2 + enable)*2^(M->index);
		aux = (dir<<1)|enable;
		aux = aux<<(M->index);
		
		PORTL &= mask; // Deja libres los bits
		PORTL |= aux;
	}
}

int cb1(){//3.5seg
	moveMotor(&motor2, DCHA);	//Quiero ponerlo listo para recibir bola
	moveMotor(&motor3, DCHA);	//avanza adelante=DCHA
	moveMotor(&motor5, DCHA);	//Sube = DCHA
	delay(3500);
	return motor3.retardo;
}

int cb2(){//1seg
	moveMotor(&motor4, DCHA);	//cierro compuerta=DCHA
	delay(1000);
	return motor4.retardo;
}

int cb3(){//3.5seg
	moveMotor(&motor5, IZDA);	//Baja motor5
	moveMotor(&motor3, IZDA);	//Retrocede motor3
	moveMotor(&motor1, IZDA);	//Lo pone abajo
	delay(3500);
	return 560;
}

int cb4(){//1.5seg
	moveMotor(&motor1, DCHA); //Dejo pongo la bola lista en el lanzador
	moveMotor(&motor2, IZDA);
	delay(18);
	stopMotor(&motor2);
	delay(2000);
	return motor1.retardo;
}

int cb5(){//1seg
	moveMotor(&motor2,IZDA); //IMPORTANTE no se puede mover el motor 1 y motor 2 al mismo tiempo
	return 10000;
}

//cargar bola
void cargarbola(){
	pulsado=0;
	if(PINL & 0b00100000){
		moveMotor(&motor2, IZDA); //Lejos de recibir bola=IZDA
		delay(motor2.retardo >> 1); // motor2.retardo >> 1 equivale a motor2.retardo*0.5
	}
	if(PINL & 0b00001000){//Verificar que cuando SW2 esta activo M1 se sube
		moveMotor(&motor1, DCHA);
		delay(motor1.retardo);
	}
	cb1();
	delay(1333);
	
	cb2();
	delay(1333);
	
	cb3();
	delay(1333);
	
	cb4();
	delay(1333);
	
	cb5();
	
	// Habilito el SW2
	cli();
	//SW2
	PCMSK0 |= 0b00100000;
	sei();
}

//	INTERRUPCIONES


// Timer 5
ISR (TIMER5_OVF_vect){ //* temporizador de 30 seg

	contador++;
	if(contador == SegDeJuego){
		contador = 0;
		ultima = 1;
		TCCR5B = 0x00; //* deshabilito el temporizador de 30 segundos, q ya se volvera a habilitar cuando se tire por primera vez la siguiente ronda.

	}
}

// Timer 4
ISR (TIMER4_OVF_vect){ //* temporizador de 0.1 segs para el parpadeo. 
	contador01++;
	
	if(contador01 == SegDeTemp){
			contador01 = 0;
			contadorparp++;		//contamos 9 veces, para que a la décima vez, se apaguen los displays, 
			if(contadorparp == 8){
				contadorparp=0;	//y la siguiente vez vuelva a empezar con los displays mostrando la puntuacion normal
				derecha = unidades;
				izquierda = decenas;
				unidades = 0b00000000;	//esto indica que no hay ninguna barrita de los displays encendida
				decenas = 0b10000000; 
			
				a=1;
			}
			
			else if (a){
				unidades = derecha;
				decenas = izquierda;
				a=0;
				}
	}
}


// Timer 3
ISR(TIMER3_OVF_vect){
	if(cont_T3 < 40){
		(cont_T3)++;
	}
	else{
		
		// Se deshabilita TIMER3
		TCCR3B = 0X00;
		// Se habilita SW2
		PCMSK0 |= 0b00100000;
		
		// Reiniciar contador
		cont_T3 = 0;
	}
}

// Timer 2
ISR(TIMER2_OVF_vect){
	if(cont_T2 == OVERFLOWS_100_MS){
		changeBit(&PORTK, 1);
		cont_T2 = 0;
	}
	else{
		(cont_T2)++;
	}
}

// Timer 1
ISR(TIMER1_OVF_vect){
	
		if(pulsado==1){
	overflowssw6++;
	
	if(overflowssw6 == OVERFLOWS_6000){
		overflowssw6=0;
		//moveMotor(&motor5,DCHA);
		cargarbola();
		//moveMotor(&motor5,IZDA);
		TCCR1B = 0x00;//Deshabilito la interrupcion temporal
		
		if (bandera1 == 1) puntuacion++;
        if (bandera2 == 1) puntuacion++;
        if (bandera3 == 1) puntuacion++;
        if (bandera4 == 1) puntuacion++;
        if (bandera5 == 1) puntuacion++;
        if (bandera6 == 1) puntuacion++;
		
		bandera1= 0; 
		bandera2= 0; 
		bandera3= 0; 
		bandera4= 0; 
		bandera5= 0; 
		bandera6= 0; 

        if (puntuacion<9) {

            un = puntuacion ;
            de = 0;

        }

        else if (puntuacion >9) {

            un= puntuacion %10;
            de = puntuacion/10;

        }

        if (ultimatirada) {

        ultimatirada = 0;
        fin = 1;
		TCCR4B = 0x01; //  habilito el temporizador del parpadeo de 0.1 segs. ESTA HABILITACION DEBE HACERSE CUANDO LA BOLA ESTA CARGADA PARA EMPEZAR UNA NUEVA RONDA
        TIMSK4 = 0x01; 


        }

         pintar(un, de); //* actualiza las unidades y decenas en "binario", codificadas segun lo de Da,Db etc.
		 PCMSK2 = 0b00000001;
		 TCCR1B = 0x00;//Deshabilito la interrupcion temporal
	}
	}
	else{
		if(cont_T0 < 6){
		(cont_T0)++;
	}
	else{
		// Deshabilitar TIMER1
		TCCR1B = 0x00;
		// Cambias direccion motor2
		moveMotor(&motor2,!motor2.dir);
		// Reiniciar contador
		cont_T0 = 0;
	}
	}
	

}


// Timer 0
ISR(TIMER0_OVF_vect){
	if (cambio) {

		//PORTD = PORTD & 0b00000000;				
		PORTD = decenas;				
	}
	else if (cambio == 0) {

		//PORTD = PORTD & 0b00000000;
		PORTD = unidades;
	}
	if (cambio) {						//*estos dos ifs no se podrian haber metido dentro de los dos anteriores? osea que despues del PORTD = PORTD | decenas, cambio = 0?
		cambio = 0;
	}
	else if (cambio == 0) {
		cambio = 1;
	}

}

// Interrupcion SW6
ISR(PCINT2_vect) {
	
	if ((PINK & 0x01 )== 0x01) {
		PCMSK2 = 0b11111100;
		pulsado=1;
		
		
		if(fin) { //* Compruebo si es la primera tirada de la siguiente ronda.

			un = 0;
			de = 0;
			puntuacion = 0;
			unidades = 0;
			decenas = 0;
			fin = 0;
			PrimeraTirada = 1;
			ultima = 0;
			TCCR4B = 0x00; //* deshabilito el temporizador de 0.1 segs, el del parpadeo

		 }	
		 
		if (PrimeraTirada == 1) { //* compruebo si es la primera lanzada para habilitar temporizador de 30 segundos.


                TCCR5A = 0X00; // WGM0...1 A 00 PORQUE QUEREMOS TRABAJAR EN MODO NORMAL
                TCCR5B = 0x01; // WGM2...3 = 0 ( MODO NO0RMAL) , CS0...3 = 001 (SIN PREESCALAD, 1X)
                TIMSK5 = 0x01; // activo con interrupcion por desbordamiento
                PrimeraTirada = 0;
        }
		
		if( ultima ) { //* Compruebo si es la ultima lanzada

			ultimatirada = 1;
		}
		
	stopMotor(&motor2);
	moveMotor(&motor4, IZDA);//abre
	TCCR1B = 0x01;//Habilito la interrupción temporal con preescalado clk/1 de 16bits
	TIMSK1 = 0x01; //Habilito la interrupción 4.5sec por overflow
	
	// Variables del swing
	cont_T0 = 0;
	cont_T2 = 0;
	cont_T3 = 0;
	cont_SW2 = 0;
	
	// Deshabilitar SW2 y timers 2 y 3 (el 0 no es necesario pues se usa para los LEDS)
	PCMSK0 &= 0b11011111;
	TCCR2B = 0x00;
	TCCR3B = 0x00;
	
	// Apagar LED
	clearBit(&PORTK,1);

	}
	

	if (((PINK >> 2) & 0x01 )== 0x00) {				//bolo 1
		PCMSK2 = 0b11111000;
		bandera1 = 1; 
		
	}
	
	if (((PINK >> 3) & 0x01 )== 0x00) {
	   PCMSK2 = 0b11110100;
	   bandera2 = 1;
	}
	
	
	if (((PINK >> 4) & 0x01) == 0x00) {
	  PCMSK2 = 0b11101100;
	  bandera3 = 1;
	}
	
	
	if (((PINK >> 5) & 0x01) == 0x00) {
	PCMSK2 = 0b11011100;
	  bandera4 = 1;
	}

	if (((PINK >> 6) & 0x01 )== 0x00) {
	PCMSK2 = 0b10111100;
	  bandera5 = 1;
	}

	if (((PINK >> 7) & 0x01 )== 0x00) {
	PCMSK2 = 0b01111100;
	  bandera6 = 1;
	
	}
	
	
}

// Interruocion SW2 => PCINT5 (PB5)
ISR(PCINT0_vect){
	// Funcionamiento del swing:
	// 1) Avanza hacia la izda
	// 2) Toca SW2_medio => Cont++
	// 3) Toca SW2_izda => Cont++
	// 4) A partir de aqui, siempre que se detecte un SW2:
	//	- Se deeshabilita SW2
	//	- Se para el motor y se espera un tiempo para el frenado
	//	- Se pone en marcha en otro sentido
	//	- Despues de un tiempo suficiente se habilita SW2
	// 5) Cuando se pulsa SW6, se reinician los valores de swing()
	
	
	// Deshabilitamos esta interrupcion
	PCMSK0 &= 0b11011111;	
	// Habilitar timer 3
	TCCR3B = 0x01;

		if(cont_SW2 == 0){
			
			//SW6
			PCMSK2 = 0x01; //Hemos activado Interrupción PCINT16 del PORTK
			//PCICR= 0b00000100;//habilitadas interrupciones grupo 2 (de la 16 a la 23)
			
			if(P_extra == 0x00){
				setBit(&PORTK,1); // Encender el LED
			}
			else{
				// Se activa el parpadeo del LED
				TCCR2B = 0x01;
			}
			
			cont_SW2 = 1;
		}
		
		else if((cont_SW2) == 1){
			// Frenar el motor
			stopMotor(&motor2);
			// Habilitar timer 1 (freno)
			TCCR1B = 0x01; //Prescalado 1 => 101		CONTAMOS 6 OVERFLOWS 50ms a 16 bits
		}
}

void inicializacion(){
	
	pintar(un, de); //* Para q los displays empiecen poniendo 00
	moveMotor(&motor5, IZDA); //Bajar=IZDA
	moveMotor(&motor4, IZDA); //Abrir=IZDA
	
	moveMotor(&motor1,DCHA);
	
	delay(40);
	moveMotor(&motor2,DCHA);
	delay(1333);
	moveMotor(&motor1, IZDA); //recibo bola=DCHA
	delay(motor5.retardo); //Quiero tiempo suficiente para bajar motor5 y cargar bola en elevador de carga
	cargarbola();
}

//	PROGRAMA PRINCIPAL

int main(void){
	
    setup();
	inicializacion();
	while(1){
	}
   
}
