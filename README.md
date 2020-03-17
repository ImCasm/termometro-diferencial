# Termometro Diferencial

Termometro diferencial para el ```PIC16F873A``` relizado en lenguaje de programación C.

## Archivo de simulación.
El archivo de simulación .pds se encuentra dentro de la carpeta montaje del proyecto

## Uso

Al iniciar la simulación se entrará a modo de configuración, donde se le pedirá la temperatura umbral.

Luego de ingresar la temperatura umbral presione la tecla ```=``` del teclado hexadecimal para aceptar y empezar la lectura de las temperaturas.

Si la temperatura que se está leyendo actualmente en el LCD supera la temperatural umbral se emitirá un mensaje de peligro por el puerto serial.


## Controles del teclado


```÷``` Permite ingresar al modo de configuración del temperatura umbral.

```=``` Si está en modo de configuración permite ingresar el valor escrito en el LCD como temperatura umbral.


```-``` Cambia a mostrar la lectura del termometro 1 (derecha).

```+``` Cambia a mostrar la lectura del termometro 2 (izquierda).


```x``` Cambia a mostrar la diferencia de temperatura entre los termometros.



## License
[MIT](https://choosealicense.com/licenses/mit/)
