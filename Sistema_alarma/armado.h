#ifndef ARMADO_H
#define ARMADO_H

#include <Arduino.h>
#include "pins.h" // Asegúrate de que este archivo exista y tenga los defines de los pines

class Armado {
private:
    unsigned long tiempoDisparo;
    unsigned long ultimoParpadeo;
    bool estadoParpadeoLED;
    
    bool sistemaArmado;
    bool alarmaDisparada;       // Estado lógico general del sistema
    bool alarmaFisicaActiva;    // Estado de los actuadores (Buzzer y LEDs de zona)
    bool advertenciaESPEnviada;
    bool sirenaActiva;
    
    uint8_t zonaViolada;

    void monitorearZonas();
    void gestionarTiempos();
    void parpadearLEDZona();
    void comunicarESP32(String mensaje);

public:
    Armado();
    
    void activarArmado();
    void desactivarArmado();
    
    void update();
    void triggerPanico(); // Llamar desde la interrupción del Botón 1 [cite: 49]
    
    bool isAlarmaDisparada();
    uint8_t getZonaViolada();
    bool isSistemaArmado();
};

#endif