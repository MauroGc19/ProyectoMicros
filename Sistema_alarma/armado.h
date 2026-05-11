#ifndef ARMADO_H
#define ARMADO_H

#include <Arduino.h>
#include "pins.h" 

class Armado {
private:
    unsigned long tiempoDisparo;
    unsigned long ultimoParpadeo;
    bool estadoParpadeoLED;
    
    bool sistemaArmado;
    bool alarmaDisparada;       
    bool alarmaFisicaActiva;    
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
    void triggerPanico(); 
    
    bool isAlarmaDisparada();
    uint8_t getZonaViolada();
    bool isSistemaArmado();
};

#endif