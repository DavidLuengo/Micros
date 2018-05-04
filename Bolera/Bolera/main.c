/*
 * Bolera.c
 *
 * Created: 23/04/2018 15:41:15
 * Author : EquipoK
 */ 

// Puerto J1:
//	Puerto B: motores 1, 3, 4, 5 (bajo: enable, alto: dir)
//	Puerto L: motor 2 (enable 0, dir 1, bk 2)
//	Puerto K: interrupts

//	LIBRERIAS
#include <avr/io.h>

//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
const uint8_t DCHA = 0x01;		// DERECHA
const uint8_t IZDA = 0x00;		// IZQUIERDA
const uint8_t ON = 0x01;		// ENABLE ON
const uint8_t OFF = 0x00;		// ENABLE OFF
const uint8_t ACT = 0x01;		// BK ACTIVO (solo motor2)
const uint8_t DEACT = 0x00;	// BK INACTIVO (solo motor2)

typedef struct{
	volatile uint8_t * port;
	uint8_t enable;
	uint8_t dir;
	uint8_t bk;
	uint8_t index; // Indica el bit que usa para su señal de enable
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

//	FUNCIONES
void setup(void){
	for(uint8_t i = 0; i < 6; i++){
		//Deshabilitar interrupciones de switch
		//Pone en marcha todos los motores hacia su posición de inicio.
		//Esperar un tiempo (suficiente para que los que estuviesen pulsando un switch ya no lo enten haciendo en teoria)
		//Habilitar interrupciones de switch
		//Cuando todos se encuentren en su posicion original se carga la primera bola
	}
	
	//Esperamos un tiempo a que todos los SW estén pulsados.
}

void moveMotor(motor* M, uint8_t direccion){
	// Esta funcion mueve en la direccion asignada un motor
	// Ejemplo:	moveMotor(motores[1],0);
	//		moveMotor(&motor1,0);
	
	uint8_t aux = 0;
	uint8_t mask = 0b00000011;
	
	
	if (M->port == &PORTL){
		M->enable = ON;
		M->bk = DEACT;	// Esto puede no estar bien
		M->dir = direccion;
		
		aux = (bk<<2)|(direccion<<1)|enable;
		
		mask = (mask<<1)|0x01;
		mask = ~mask;
		
		//PORTL &= mask;
		//PORTL |= aux;
		
	}
	else if (M->port == &PORTB){
		M->dir = direccion;
		M->enable = ON;
		
		mask = mask<<(M->index);
		mask = ~mask;
		
		//aux = (direccion*2 + enable)*2^(M->index);
		aux = (direccion<<1)|enable;
		aux = aux<<(M->index);
		
		//PORTB &= mask; // Deja libres los bits
		//PORTB |= aux;
	}
	
	M->port* &=mask;
	M->port* |= aux;
	
}

void Dinamicstop(void){
	//Parada dinamica para motor2
	motores[2]->enable = OFF;
	motores[2]->bk = ACT;
	motores[2]->dir = ~motores[2]->dir;
}

//	INTERRUPCIONES

// Antirrebotes

// Switch pulsado



//	PROGRAMA PRINCIPAL
int main(void)
{
    setup();
	
	
    while (1) 
    {
	    // Bucle principal
    }
}

