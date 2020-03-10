#include <xc.h>
#define _XTAL_FREQ 4000000 //Frecuencia del micro
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int waiting = 1; //Bandera de espera para ingresar temp maxima
unsigned int showDiferenceTemp = 0; // Bandera para saber si se debe mostrar la diferencia de temperaturas
unsigned int maxTemp = 150; // Valor de la temperatura max
unsigned int onConfTempMode = 1; //bandera de modo configuracion
char maxTempString[3]; // Valor de la temperatura max en string
unsigned int cont = 0; //Contador del puntero del LCD
unsigned int clear = 1; //Bandera para saber si se puede limpiar el mensaje de inicio -> (Ingrese temp MAX)


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
    // 1. Como solo se necesita leer la mitad del puerto B (RB2,RB3,RB4,RB5) se hace shift right de los primeros 2 digitos
    // para ignorar los ultimos 2 bits
    int d1 = n >> 2; 
    PORTC = d1;
    RC0 = rs;
    flank();
    // 2. Luego shift left para ignorar los primeros 2 bits
    int d2 = n << 2;
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
    sendNibble(1, 0); //se le envia 1 al D0 de la pantalla LCD para limpiar y volver el puntero al inicio
}

/**
 * Configuracion inicial del termometro
 */
void configThermo() {
    TRISC = 128; //SALIDA
    TRISA = 1; //ENTRADA
    ADCON1 = 128; //ANALOGOS => ADFM:1->DERECHA, ADCS2:0->FRECUENCIA, -, -, (DIGITAL/ANALOGO{VER HOJA DE DATOS -ADCON1}))
    ADCON0 = 1; //(ADCS1:0,ADCS0)->FRECUENCIA; (CH2:0,CH1:0,CH2:0)->PUERTO EN DONDE ESTA CONECTADO EL TERMO; GO/DOBE:0->SIN LEER; -; ADON:1 ENCENDIDO    
    ADCS0 = 1; //hasta el 2 para la frecuencia        
    sendNibble(8, 0); //Se le manda el nibble 
}

/**
 * Configuracion inicial del teclado
 */
void configKeyboard() {
    OPTION_REG = 7; //ACTIVAR LAS RES
    TRISB = 240; //llevo el 240 del acumulador para establecer los 4 menos sig B como salida, y los 4 más como entrada 00001111  	
    RBIE = 1; //activo las int (RBIE, GIE, RBIF)
    GIE = 1; //Activar todas las interrupciones
    RBIF = 1; //Activar interrupciones del RB4 a RB7.
}

/**
 * Configuracion del puerto serial
 */
void configSerialPort() {
    TRISC = 0; //C COMO SALIDA 
    SPEN = 1; //HABILITA EL PUERTO SERIAL
    CREN = 0; // HABILITADOR DE RECEPCION 0 -> DISABLE
    TXEN = 1; //0 TRANSMITE - 1 NO TANSMITE
    SYNC = 0; // 0 ASINCRONA 1 SINCRONA
    BRGH = 1; // TRANSMISION DE ALTA VELOCIDAD 1 -> ENABLE

    TXIE = 1; // HABILITADOR DE INTERRUPCION DE LA TRANSMISION
    RCIE = 0; // HABILITADOR DE INTERRUPCION DE RECEPCION

    SPBRG = 25.04166667;
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
 * @return termperatura del termometro elegido en formato digito entero
 */
int getTemperatureInt() {
    int temp;
    temp = getVoltaje()*0.4887585532746823;

    return temp; //devuelve entero (para comparaciones)
}

/**
 * Obtiene la temperatura del sensor 1 o 2 según el canal que esté activo
 * @param thermometer 1 si es quiere temperatura del termo 1, 2 si quiere la temp del termo 2
 * @return termperatura del termometro elegido en formato cadena de texto
 */
char getTemperatureString() {
    int temp;
    temp = getVoltaje()*0.4887585532746823;
    char str[12];
    sprintf(str, "%d", temp);
    strcat(str, "'C");

    return str; //devuelve cadena
}

/**
 * Secuencia de ceros en el puerto b (Para activar interrupcion)
 */
void ZeroSequenceKeyboard() {
    PORTB = 254; //11111110
    PORTB = 253; //11111101
    PORTB = 251; //11111011
    PORTB = 247; //11110111
}

/**
 * Limpia el LCD si se escribe la primera tecla en MODO CONFIGURACION DE TEMP
 */
void clearIfFirst() {
    if (clear == 1) {
        clearLCD();
        clear = 0; //bandera se vuelve cero luego de oprimir el primer numero
    }
}

/**
 * Permite cambiar de canal para leer entre los 2 termometros
 * @param channel canal al que se desea pasar, 1 -> canal 0, 2-> canal 1
 */
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
            if (waiting) { //si esta en espera (modo conf)
                clearIfFirst(); // si es la primera tecla que oprime borra el mensaje que hay en LCD
                if (cont < 3) { //si no se han apretado mas de 3 digitos
                    maxTempString[cont] = '7'; //concatena a la temp maxima
                    putCharacter('7', cont); //pone el caracter en el LCD
                    cont++; //incrementa posicion del puntero en LCD
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
            showDiferenceTemp = 1;
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
            showDiferenceTemp = 0;
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
            showDiferenceTemp = 0;
            changeThermometerChannel(2);
            break;
    }
}

/**
 * Funcion de interrupcion
 */
void __interrupt() globalInterruption(void) {

    if (INTCONbits.RBIF) {

        keyboard(); // evento para leer los las teclas oprimidas
        __delay_ms(200); //delay para no encadenar mas de un evento en la interrupcion

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

    while (waiting == 1) { //Si esta en modo de configuracion espera hasta que se ingresen los digitos
        ZeroSequenceKeyboard(); //Secuencia de ceros que se mueve por el puerto B
    }

    if (maxTemp > 150) { //Si la temperarura que se ingresa es mayor a la que puede mostrar el micro, se deja predetemrinada a 150
        maxTemp = 150;
    }
}

/**
 * Permite saber si esta alta la temperatura respecto al umbral
 * @param temp temperatura que se va a evaluar
 * @return si sobrepasa el umbral 1, 0 de lo contrario
 */
int isItHot(int temp) {
    return temp > maxTemp;
}

/**
 * Permite enviar un mensaje de alerta por el puerto serial del micro
 * @param message mensaje que se va a enviar
 */
void turnOnSerialAlarm(char message[]) {
    for (int i = 0; i < strlen(message); i++) {
        while (!TXIF);
        TXREG = message[i];
    }
}

/**
 * Muestra la diferencia de temperatura de los termometros
 */
void showDifferenceTemp() {

    char tempDifference[5];

    changeThermometerChannel(1); //Cambio de canal al 0
    __delay_ms(200);
    int temp1 = getTemperatureInt(); //Leo la temperatura del canal 0
    
    changeThermometerChannel(2); // Cambio de canal al 1
    __delay_ms(200);
    int temp2 = getTemperatureInt(); // Leo la temperatura del canal 1

    if (temp1 > temp2) { // Saco la diferencia de las temperaturas
        sprintf(tempDifference, "%d", (temp1 - temp2));
    } else {
        sprintf(tempDifference, "%d", (temp2 - temp1));
    }

    clearLCD(); //Limpio el LCD
    writeOnLCD(tempDifference); //Escribo diferencia en LCD
}

/**
 * Muestra la temperatura y si es mayor que el umbral envia mensaje de alarma por el puerto serial
 */
void showTemp() {
    
    int temp = getTemperatureInt(); //Lee temperatura

    if (isItHot(temp)) { //Si esta caliente (mayor al umbral) envia alarma

        char msg[12] = "PELIGRO "; //Mensaje alarma
        turnOnSerialAlarm(msg); //Envio por el puerto serial
    }
    writeOnLCD(getTemperatureString()); //Se escribe en LCD la temperatura que haya
    ZeroSequenceKeyboard(); //Sigue la secuencia de ceros
}

void main(void) {
    // CONFIGURACIONES INICIALES //
    configThermo();
    configKeyboard();
    configSerialPort();

    int running = 1;

    //Segun las banderas activadas realizara una accion
    while (running) {
        if (onConfTempMode) {
            configTempMode(); //  MODO DE CONFIGURACION TEMPERATURA UMBRAL  //
            onConfTempMode = 0;
        } else if (showDiferenceTemp) {
            showDifferenceTemp(); //  DIFERENCIA DE TEMPERATURA  //
        } else {
            showTemp(); //  MUESTRA TEMPERATURAS  //
        }
    }
    wait();

    return;
}