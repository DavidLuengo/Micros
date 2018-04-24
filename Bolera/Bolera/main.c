/*
 * Bolera.c
 *
 * Created: 23/04/2018 15:41:15
 * Author : EquipoK
 */ 

//	LIBRERIAS
#include <avr/io.h>

//	DEFINICIONES y ESTRUCTURAS
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

motor *motores[] = {&motor1, &motor2, &motor3, &motor4, &motor5};

//	FUNCIONES
void setup(){
	
}

void moveMotor(motor* M, uint8_t direccion){
	// Esta funcion mueve en la direccion asignada un motor
	M->enable = 1;
	M->bk = 0;	// Esto puede no estar bien
	M->dir = direccion;
}

//	PROGRAMA PRINCIPAL
int main(void)
{
    setup();
	
	
    while (1) 
    {
    }
}

