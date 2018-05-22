;Parte en ensamblador DavidTito
;.cseg .dseg .org .equ
;--FUNCIONES:En rutina de interrupción que C maneje la gsetión del vector de interrupción y  llamar a func en asm
;--VARIABLES:Para variables globales dejar que the C code manage the variables (direcc mem y tal)and pass parameters to the assembly code either by value or by reference
;declaras en C e inicializas en la parte de vbles globales fuera de func al ppo, y para que el asm la vea pones .extern nombrevble en el código asm
;--REGISTROS Y PASO DE PARAM: ver pdfs para el uso de registros ver tabla (generalmte guardar en pila y luego restaurar al acabar func asm pero alguno poner a 0)
;para paso de param se hace con los registros de la lista por pares incluso un char, si no caben ocupa más de 2bytes se pasan por la pila
;ver paso de param mediante pila en apuntes PAG33, con direcc indexado el SPL Y SPH con IN/OUT, RET devuelve PC pero tengo que limpiar pila en código ppal


;para que vea las vbles globales de C, el comp sustituirá nombre vble por direcc mem parece según diapos de Mix
extern cont_T3

;-----------------Rutina Timer 3 en ASM
asm T3_ASM:
;salvo registros
	push r10
	push r16
	in r16, SREG   ;guardo registro de estado se accede como E/S dedicada; Aunque es necesario? no es interrupt pero por si acaso
	push r16

;Ojo no es bucle porque solo entro tras desbordarse cada vez el timer y entonces incremento contador
	ldi r16,40        ;ver porque quiero para cont_T3<40, ver si es 39 o 40 que me lío
	lds r10, cont_T3
	sub r16, r10
	brne Incremento  ;si no son iguales incremento contador y Fin rutina

; si son iguales pongo contador a 0, habilita Timer3, habilita SW2 y Fin rutina
	ldi r16,0x00   
	sts cont_T3, r16
;Todos los reg de Interrup y timers se accede como posic memoria donde compilador sustit nombre por direcc mem; no como reg E/S IN OUT como en PORTX DDRx PINx
; Se deshabilita TIMER3
	ldi r16,0x00 ;ya lo estaba  
	sts TCCR3B, r17
; Se habilita SW2, con máscara OR pone a 1 el bit PCINT5 del SW2 sin modificar el resto, OPERAC arit y lógicas solo con registros GPR!!! no direct memoria
	lds r16, PCMSK0
	ori r16, 0b00100000
	sts PCMSK0,r16
	
	rjmp Fin	

Incremento:
	inc r10
	sts cont_T3,r10
	rjmp Fin
    
;limpio pila para que no se me acumule tras iteraciones
	POP r16  ;lo último en pila es reg estado, va en orden inverso a como metí
	out SREG,r16
	POP r16
	POP r10

Fin: 
ret
;ahora la direcc de vuelta ya se habrá sacado de la pila 




;-----------------------------------------------------------------
;FUNCIÓN SETUP EN ASM


asm setup_asm:
	;Pone en marcha todos los motores hacia su posición de inicio.
	;Esperar un tiempo
	;Cuando todos se encuentren en su posicion original se carga la primera bola


	;aquí no tengo que guardar registros ni nada solo poner a cero por si acaso todos
	; SREG?   
	ldi r17,0x00
	ldi r18,0x00
	ldi r19,0x00

	;Reg E/S DDRx,PORTx,PINx, se puede acceder con IN/OUT a todos creo
	ldi r17,0xff
	out DDRL, r17       ;todos salidas motor 1, motor 3, motor 4, motor 5 enable y dirección
	out	DDRD,r17	;todos salida para el display
	ldi r17,0b00000010
	out DDRL, r17       ;PK1 salida Led, el resto entrada interrupciones sensores opticos
	ldi r17,0b00000111
	out DDRL, r17		;PB0,PB1, PB2 salidas para el motor 2 y PB3 => SW1, PB4 => SW3, PB5 => SW2, PB6 => SW5, PB7 => SW4
	
	
	;LED apagado al comienzo, entiendo que es activo por nivel alto---PORTK &=11111101;
	in r17, PORTK
	andi r17, 0b11111101
	out PORTK, r17 
	;se podía haber hecho con cbi PORTK,B que es instrucc atómica evito ciclo RMW el operando puerto tiene que ser REG E/S
	;EH pero entonces las funciones setBit y clear Bit así salen solas!!!!! Sí pero va por paso de param con puntero...
	
	
	
	;Todos los timers Func normal y por overflow
	cli

	
	;ver si hay que poner la direcc de memoria de cada TCCR0A de la hoja de catálogo o así vale según código Cris debería
	;sí porque son macros, y aquí no hay duda de si sts o out porque no es E/S
	ldi r18,0x00
	sts TCCR0A,r18
	sts TCCR0B,r18   ;Deshabilitado, el que deshabilita es el B con prescalado 000
	
	sts TCCR2A,r18
	sts TCCR2B,r18   ;Deshabilitado	

	sts TCCR1A,r18
	sts TCCR1B,r18   ;Deshabilitado	
	
	sts TCCR3A,r18
	sts TCCR3B,r18   ;Deshabilitado	
	
;voy a poner int timers por overflow	
	ldi r19,0x01
	
	sts TIMSK0,r19    ;Timer 0  se usa para...2 cosas
	sts TIMSK2,r19
	sts TIMSK1,r19   ;para dos cosas al final
	sts TIMSK3,r19
	sts TIMSK4,r19   ;debería ser para dos cosas, sugerencia
	sts TIMSK5,r19

	
	;habilitadas interrupciones grupo 2 (de la 16 a la 23) y el grupo 0 (de la 0 a la 7)--101
	ldi r17, 0b00000101
	sts PCICR,r17

	;desactivo todas ellas con máscaras
	sts PCMSK0,r18
	sts PCMSK2,r18
	
	sei
;es setup no he puesto nada en pila, salvo direcc pero va solo

ret
;limpita



