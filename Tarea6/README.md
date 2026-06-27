# Informe del simulador implementado

## Descripción
Tarea 6 de Arquitectura de computadoras: Un simulador funcional de la arquitectura RISC-V de 32 bits. El programa permite cargar binarios, ejecutar instrucciones paso a paso e inspeccionar el estado del procesador.

## Lenguaje
* **Lenguaje:** C++

## Características Principales
* **Ciclo de Instrucción:** Implementación del ciclo *Fetch-Decode-Execute*.
* **Interfaz:** Interfaz interactiva de línea de comandos
* **Comandos soportados:**
    * `step`: Ejecuta la siguiente instrucción
    * `pc`: Muestra el valor actual del contador de programa
    * `regs <xN>`: Muestra el valor de registros específicos
    * `exit`: Finaliza la simulación

## Instrucciones de Uso

### Compilación
Para compilar el simulador utilizar el siguiente comando en la terminal:
```bash
g++ -O3 main.cpp -o awesome-simulator
```
Luego de eso usar 
```bash
./awesome-simulator programa.bin
```
Son implementaciones de 
lui x8, 0x12345
addi x9, x0, 5
ori x10, x9, 10
slli x11, x10, 4

Y se verifica con
```bash
regs x8 x9 x10 x11
```

