/*
-Guardar cambios de Timer0 a Timer 1 para el brake del swing y no pisar el selector de display. Ahora timer 1 es compartido, primero para espera brake swing 
y después
-El timer 4 para parpadeo puntuación tras partida extra se podría liberar usando el Timer2 ya que el parpadeo del LED se deja de hacer al tirar por última vez y 
ese parpadeo es posterior es cuestión de poner una variable para distinguir en la rutina  
Y usar el Timer 4 para todos los delays secuenciales DE CARGAR BOLA con diferentes contadores banderas. Es posible??
-Ver si han llamado a la vble de p_extra para LED así o cómo
-HECHO:Vbles contadores a 16 bits porque hasta 8 solo contaba hasta 255 
*/


//prototipo ensamblador
extern void T3_asm ();
extern void setup_asm();

//código en c a sustituir
ISR(TIMER3_OVF_vect){
T3_ASM;
}

//código en c a sustituir llamada a setup en el main 
setup_asm;


/*
// Timer 3 SW2 largo
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

-----------------------
//23 líneas netas de ASM salen
//Se puede Hacer la de setBit o más complicado changeBit en ASM, pero ver paso de param de Puerto con puntero??? y bit
//Se puede hacer el setup en asm que es solo config reg de timers e interrupciones
*/
/*
void setup(void){
	//Pone en marcha todos los motores hacia su posición de inicio.
	//Esperar un tiempo
	//Cuando todos se encuentren en su posicion original se carga la primera bola

	
	DDRL=0xff;			//todos salidas motor 1, motor 3, motor 4, motor 5 enable y dirección
	DDRK=0b00000010;	//PK1 salida Led, el resto entrada interrupciones sensores opticos
	DDRB=0b00000111;	//PB0,PB1, PB2 salidas para el motor 2
						//PB3 => SW1, PB4 => SW3, PB5 => SW2, PB6 => SW5, PB7 => SW4
	DDRD=0xff;			//todos salida para el display
	
	//LED apagado al comienzo, entiendo que es activo por nivel alto
	PORTK &=11111101;
	
	
	//Todos los timers Func normal y por overflow
	//El 0 para;el 2 para; el 1 para; el 3 para;el 4 para; el 5 para;
	cli();	
	
	TCCR0A = 0x00; 
	TCCR0B = 0x00; // Deshabilitado
	TIMSK0 = 0x01;
	
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
*/