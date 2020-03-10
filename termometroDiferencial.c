#include <xc.h>
#define _XTAL_FREQ 4000000 //Frecuencia del micro
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int waiting = 1; //Bandera de espera para ingresar temp maxima
int maxTemp = 150; // Valor de la temperatura max
int inConfTempMode = 1; //bandera de modo configuracion
char maxTempString[3]; // Valor de la temperatura max en string
int cont = 0;
int clear = 1;

/**
 * No permite que se acabe el programa
 */
void wait() {
    while (1);
}

/**
 * PRODUCE EL FLANCO DE BAJADA
 */
void flank() {
    RC1 = 1;
    __delay_ms(1);
    RC1 = 0;
}

/**
 * ENVIA UN NIBBLE DE A 4 BITS 
 * @param data dato que va a enviar 
 * @param rs Si es rs 0 para configurar o 1 para mostrar
 */
void sendNibble(int n, int rs) {
    int d1 = n >> 2;
    //    d1 = d1 & 60;
    PORTC = d1;
    RC0 = rs;
    flank();
    int d2 = n << 2;
    //    d2 = d2 & 60;
    PORTC = d2;
    RC0 = rs;
    flank();
}

/**
 * Enciende el LCD
 */
void turnOnLCD() {
    int turnOn = 14;
    int d1 = turnOn >> 2;
    //    d1 = d1 & 60;
    PORTC = d1;
    RC0 = 0;
    flank();
    int d2 = turnOn << 2;
    //    d2 = d2 & 60;
    PORTC = d2;
    RC0 = 0;
    flank();
}

/**
 * Pinta un caracter en el LCD
 * @param character caracter a pintar
 * @param position posicion en que se va a pintar en pantalla
 */
void putCharacter(char character, int position) {
    //Seleccion posicion RAM
    sendNibble(128 + position, 0);
    //Escribir ram
    sendNibble(character, 1);
    //Encender LCD
    turnOnLCD();
}

/**
 * Escribe una palabra en el LCD
 * @param word palabra a escribir
 */
void writeWord(char word[]) {
    for (int i = 0; i < strlen(word); i++) {
        putCharacter(word[i], i);
    }
}

/**
 * Escribe en el LCD 
 * @param word palabra a escribir
 */
void writeOnLCD(char word[]) {
    writeWord(word);
}

/**
 * Limpia la pantalla del LCD
 */
void clearLCD() {
    sendNibble(1, 0);
}

/**
 * Configuracion inicial
 */
void configThermo() {
    TRISC = 128; //SALIDA
    TRISA = 1; //ENTRADA
    ADCON1 = 128; //ANALOGOS => ADFM:1->DERECHA, ADCS2:0->FRECUENCIA, -, -, (DIGITAL/ANALOGO{VER HOJA DE DATOS -ADCON1}))
    ADCON0 = 1; //(ADCS1:0,ADCS0)->FRECUENCIA; (CH2:0,CH1:0,CH2:0)->PUERTO EN DONDE ESTA CONECTADO EL TERMO; GO/DOBE:0->SIN LEER; -; ADON:1 ENCENDIDO    
    ADCS0 = 1; //hasta el 2 para la frecuencia        
    sendNibble(8, 0); //Se le manda el nibble 
}

void configKeyboard() {
    OPTION_REG = 7; //ACTIVAR LAS RES
    TRISB = 240; //llevo el 240 del acumulador para establecer los 4 menos sig B como salida, y los 4 más como entrada 00001111  	
    RBIE = 1; //activo las int (RBIE, GIE, RBIF)
    GIE = 1; //Activar todas las interrupciones
    RBIF = 1; //Activar interrupciones del RB4 a RB7.
}

/**
 * Obtiene el valor del sensor en voltaje
 * @return voltaje sensor
 */
int getVoltaje() {
    GO = 1;
    while (GO) {
    };
    return ADRESH * 256 + ADRESL;
}

/**
 * Obtiene la temperatura del sensor 1 o 2 según el canal que esté activo
 * @param thermometer 1 si es quiere temperatura del termo 1, 2 si quiere la temp del termo 2
 * @return termperatura del termometro elegido
 */
char getTemperature() {
    int temp;
    temp = getVoltaje()*0.4887585532746823;
    char str[12];
    sprintf(str, "%d", temp);
    strcat(str, "'C");
    return str;
}

/**
 * Secuencia de ceros en el puerto b (Para activar interrupcion)
 */
void ZeroSequenceKeyboard() {
    PORTB = 254;
    PORTB = 253;
    PORTB = 251;
    PORTB = 247;
}

/**
 * Limpia el LCD si se escribe la primera tecla en MODO CONFIGURACION DE TEMP
 */
void clearIfFirst() {
    if (clear == 1) {
        clearLCD();
        clear = 0;
    }
}

void changeThermometerChannel(int channel) {
    if (channel == 2) {
        CHS0 = 1;
    } else {
        CHS0 = 0;
    }
}

/**
 * Funcion que escucha los eventos del teclado.
 */
void keyboard() {

    switch (PORTB) {
        case 238: //7
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '7';
                    putCharacter('7', cont);
                    cont++;
                }
            }
            break;
        case 222: //8
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '8';
                    putCharacter('8', cont);
                    cont++;
                }
            }
            break;
        case 190://9
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '9';
                    putCharacter('9', cont);
                    cont++;
                }
            }
            break;
        case 126:

            break;
        case 237://4
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '4';
                    putCharacter('4', cont);
                    cont++;
                }
            }
            break;
        case 221://5
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '5';
                    putCharacter('5', cont);
                    cont++;
                }
            }
            break;
        case 189: // 6
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '6';
                    putCharacter('6', cont);
                    cont++;
                }
            }
            break;
        case 125:

            break;
        case 235: // 1
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '1';
                    putCharacter('1', cont);
                    cont++;
                }
            }
            break;
        case 219: // 2
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '2';
                    putCharacter('2', cont);
                    cont++;
                }
            }
            break;
        case 187: // 3
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '3';
                    putCharacter('3', cont);
                    cont++;
                }
            }
            break;
        case 123: // -
            changeThermometerChannel(1);
            break;
        case 231:
            
            break;
        case 215: //0
            if (waiting) {
                clearIfFirst();
                if (cont < 3) {
                    maxTempString[cont] = '0';
                    putCharacter('0', cont);
                    cont++;
                }
            }
            break;
        case 183: // =
            maxTemp = atoi(maxTempString);
            waiting = 0;
            break;
        case 119:// +
            changeThermometerChannel(2);
            break;
    }
}

/**
 * Funcion de interrupcion
 */
void __interrupt() globalInterruption(void) {

    if (INTCONbits.RBIF) {

        keyboard();
        __delay_ms(200);

        INTCONbits.RBIF = 0;
    }

    //GIE = 1;
}

/**
 * Modo configuracion para configurar temperatura UMBRAL
 */
void configTempMode() {

    writeOnLCD("Ingrese temp MAXIMA");
    writeOnLCD("Ingrese temp MAXIMA");

    while (waiting == 1) {
        ZeroSequenceKeyboard();
    }

    if (maxTemp > 150) {
        maxTemp = 150;
    }
}

void main(void) {
    // CONFIGURACIONES INICIALES //
    configThermo();
    configKeyboard();
    //        CONFIGURACION TEMP     //
    configTempMode();

    int readTemp = 1;
    while (readTemp) {
        writeOnLCD(getTemperature());
        ZeroSequenceKeyboard();
    }

    wait();

    return;
}