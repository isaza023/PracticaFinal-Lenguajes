# Práctica 2 - Motor Integrado de Itinerarios y Tarifas

**Integrantes:** Samuel Gómez, Samuel Isaza
**Curso:** SI1002-7 Lenguajes y Paradigmas de Computación, EAFIT 2026-2

## Cómo compilar y ejecutar

    g++ -std=c++17 -Wall -Wextra main.cpp -o main
    ./main

## Pregunta 1: ¿Qué pasa si `TransportMode` no tiene destructor virtual?

`Network` destruye los medios a través de punteros a la base. Sin destructor
virtual, borrar un hijo por un puntero al padre es comportamiento indefinido:
solo corre el destructor de la base y se omite el del hijo, lo que puede causar
fugas de memoria. Con el destructor virtual, se ejecuta primero el del hijo y
luego el de la base.

## Pregunta 2: ¿Por qué una clase abstracta y no un enum con switch?

Con un enum, cada decisión por tipo es un `switch` repartido por el programa, y
agregar un medio obliga a modificarlos todos. Con una clase abstracta, cada
medio lleva su comportamiento en una sola clase: un medio nuevo es una clase
nueva y no se toca nada más. Además, `chargeItinerary` nunca pregunta qué es un
medio, solo consulta `allowsIntegration()` u `opensIntegrationWindow()`.
