#include <avr/io.h>
#include <avr/interrupt.h>
#define SegDeJuego (3662)
#define SegDeTemp (13)



uint8_t contador = 0;			//*habrá que poner a 0 el contador en algun momento no? que es uint8_t?
uint8_t contador01 = 0;
int PrimeraTirada = 1,
bandera1= 0, bandera2= 0, bandera3= 0, bandera4= 0,bandera5= 0, bandera6= 0, contadorparp =0, a=0,
puntuacion= 0, unidades = 0, decenas= 0, cambio = 1, ultima= 0, ultimatirada = 0, fin= 0, derecha = 0, izquierda = 0;

void pintar(int unidades, int decenas) {

    switch (unidades) {

    case 0 :  unidades = 0b11111110;


    case 1 :  unidades = 0b01100000;


    case 2 :  unidades = 0b11011010;


    case 3 :  unidades = 0b11110010;

    case 4 :  unidades = 0b01100110;


    case 5 :  unidades = 0b10110110;


    case 6 :  unidades = 0b00111110;


    case 7 :  unidades = 0b11100000;


    case 8 :  unidades = 0b11111110;


    case 9 :  unidades = 0b11100110;
	
	default:  unidades = 0b00000000;  //*display apagado durante el parpadeo


}
    switch (decenas) {

    case 0 :  decenas = 0b11111111; 


    case 1 :  decenas = 0b01100001;


    case 2 :  decenas = 0b11011011;


    case 3 :  decenas = 0b11110011;

    case 4 :  decenas = 0b01100111;


    case 5 :  decenas = 0b10110111;


    case 6 :  decenas = 0b00111111;


    case 7 :  decenas = 0b11100001;


    case 8 :  decenas = 0b11111111;


    case 9 :  decenas = 0b11100111;
	
	default : decenas = 0b00000000; //* display apagado durante el parpadeo
	
}



ISR (TIMER4_OVF_vect){ //* temporizador de 0.1 segs para el parpadeo
	contador01++;
	if(contador01 == SegDeTemp){
			contador01 = 0;
			contadorparp++;			//contamos 9 veces, para que a la décima vez, se apaguen los displays, 
			if(contadorparp == 8){	//y la siguiente vez vuelva a empezar con los displays mostrando la puntuacion normal
				contadorparp = 0;
				derecha = unidades;
				izquierda = decenas;
				unidades = 10;	//esto indica que no hay ninguna barrita de los displays encendida
				decenas = 10; 
				a=1;
			}
			else if (a){
				unidades = derecha;
				decenas = izquierda;
				a=0;
				}
	}
}



ISR (TIMER5_OVF_vect){ //* display de 30 seg

	contador++;
	if(contador == SegDeJuego){
		contador = 0;
		ultima = 1;
		TCCR5B = 0x00; //* deshabilito el temporizador de 30 segundos, q ya se volvera a habilitar cuando se tire por primera vez la siguiente ronda.

	}
}
ISR (TIMER0_OVF_vect){ //* temporizador display

    //* Aqui Julio tienes q añadir en los ifs q parpadeo sea 0 , y crear otros ifs en los q si parpadeo es 1 hagan lo de 0.9s y 0.1s,
    //* preguntales a estos como se hace.


	
	if (cambio) {

		PORTD = PORTD & 0b00000000;				
		PORTD = PORTD | decenas;				
	}
	else if (cambio == 0) {

		PORTD = PORTD & 0b00000000;
		PORTD = PORTD | unidades;
	}
	if (cambio) {						//*estos dos ifs no se podrian haber metido dentro de los dos anteriores? osea que despues del PORTD = PORTD | decenas, cambio = 0?
		cambio = 0;
	}
	else if (cambio == 0) {
		cambio = 1;
	}



}

 


ISR(PCINT2_vect){ 
	
	if ((PINK >> 1) & 0x01 == 0x01) {
		
		bandera1 = 1; 
	}
	
	if ((PINK >> 2) & 0x01 == 0x01) {
	
	bandera2 = 1;
	}
	
	
	if ((PINK >> 3) & 0x01 == 0x01) {
	
	bandera3 = 1;
	}
	
	
	if ((PINK >> 4) & 0x01 == 0x01) {
	
	bandera4 = 1;
	}

	if ((PINK >> 5) & 0x01 == 0x01) {
	
	bandera5 = 1;
	}

	if ((PINK >> 6) & 0x01 == 0x01) {
	
	bandera6 = 1;
	}
	
}
	

void setupdisplay() {
    PORTD = unidades; //*inicializo el display en las unidades
    TCCR0B = 0x01; // *Empieza a contar el temporizador del display q alterna unidades y decenas a 8192 microsegundos.
    TIMSK0 = 0x01; //

}

 //*----------------a meter en el temporizador de 4.5 seg de Cris e Ibra, q se activa 4.5 seg despues de lanzar cada bola--------- :

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

            unidades = puntuacion ;
            decenas = 0;

        }

        else if (puntuacion >9) {

            unidades= puntuacion %10;
            decenas = puntuacion/10;

        }

        if (ultimatirada) {

        ultimatirada = 0;
        fin = 1;
		TCCR4B = 0x01; //  habilito el temporizador del parpadeo de 0.1 segs. ESTA HABILITACION DEBE HACERSE CUANDO LA BOLA ESTA CARGADA PARA EMPEZAR UNA NUEVA RONDA
        TIMSK4 = 0x01; 


        }

         pintar(unidades, decenas); //* actualiza las unidades y decenas en "binario", codificadas segun lo de Da,Db etc.

//* --------------------------------HASTA AQUI------------------------------------


int main () {
	PCICR = 0b00000100; //* configuracion de bola lanzada creo.
    PCMSK2 = 0b01111110;
	
     DDRK = 0x00; //*Inicializa puerto K como entrada para los bolos
	 DDRD = 0xFF; //* Iniciliaza puerto D como salida para los displays



pintar(unidades, decenas); //* Para q los displays empiecen poniendo 00

setupdisplay();


while (1) {
	


    if( (PINK%2) && ultima == 0) { //* Bola lanzada

        if (PrimeraTirada == 1) { //* compruebo si es la primera lanzada para habilitar temporizador de 30 segundos.


                TCCR5A = 0X00; // WGM0...1 A 00 PORQUE QUEREMOS TRABAJAR EN MODO NORMAL
                TCCR5B = 0x01; // WGM2...3 = 0 ( MODO NO0RMAL) , CS0...3 = 001 (SIN PREESCALAD, 1X)
                TIMSK5 = 0x01; // activo con interrupcion por desbordamiento
                PrimeraTirada = 0;
        }
    }

    if( (PINK%2) && ultima ) { //* Compruebo si es la ultima lanzada

       ultimatirada = 1;
    }

    if( (PINK%2) && fin) { //* Compruebo si es la primera tirada de la siguiente ronda.

        unidades = 0;
        decenas = 0;
        puntuacion = 0;
        unidades = 0;
        decenas = 0;
        fin = 0;
        PrimeraTirada = 1;
		ultima = 0;
		TCCR4B = 0x00; //* deshabilito el temporizador de 0.1 segs


       
    }

}

}




