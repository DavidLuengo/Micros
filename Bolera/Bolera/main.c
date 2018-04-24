/*
 * Bolera.c
 *
 * Created: 23/04/2018 15:41:15
 * Author : EquipoK
 */ 

//	LIBRERIAS
#include <avr/io.h>

//	DEFINICIONES, ESTRUCTURAS y CONSTANTES
const uint8_t DCHA = 1;		// DERECHA
const uint8_t IZDA = 0;		// IZQUIERDA
const uint8_t ON = 1;		// ENABLE ON
const uint8_t OFF = 0;		// ENABLE OFF
const uint8_t ACT = 1;		// BK ACTIVO (solo motor2)
const uint8_t DEACT = 0;	// BK INACTIVO (solo motor2)

typedef struct{
	volatile uint8_t * port;
	uint8_t enable;
	uint8_t dir;
	uint8_t bk;
	//A lo mejor hay que añadir una variable de velocidad
} motor;

//	VARIABLES
motor motor1;
motor motor2;
motor motor3;
motor motor4;
motor motor5;

	// Vector de direcciones a los motores
motor *motores[] = {&motor1, &motor2, &motor3, &motor4, &motor5};

//	FUNCIONES
void setup(void){
	
}

void moveMotor(motor* M, uint8_t direccion){
	// Esta funcion mueve en la direccion asignada un motor
	// Ejemplo:	moveMotor(motores[1],0);
	//		moveMotor(&motor1,0);
	M->enable = ON;
	M->bk = DEACT;	// Esto puede no estar bien
	M->dir = direccion;
}

void Dinamicstop(){
	//Parada dinamica para motor2
}

//	PROGRAMA PRINCIPAL
int main(void)
{
    setup();
	
	
    while (1) 
    {
    }
}

