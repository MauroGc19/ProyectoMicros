#include "armado.h"

const unsigned long TIEMPO_ESPERA_ESP = 30000; 
const unsigned long INTERVALO_PARPADEO = 150;  

Armado::Armado() {
    sistemaArmado = false;
    alarmaDisparada = false;
    alarmaFisicaActiva = false;
    advertenciaESPEnviada = false;
    sirenaActiva = false;
    zonaViolada = 0;
    tiempoDisparo = 0;
    ultimoParpadeo = 0;
    estadoParpadeoLED = false;
}

void Armado::activarArmado() {
    sistemaArmado = true;
    alarmaDisparada = false;
    alarmaFisicaActiva = false;
    advertenciaESPEnviada = false;
    sirenaActiva = false;
    zonaViolada = 0;
    
    digitalWrite(PIN_LED_ARMED, HIGH); 
}

void Armado::desactivarArmado() {
    sistemaArmado = false;
    alarmaDisparada = false;
    alarmaFisicaActiva = false;
    sirenaActiva = false;
    
    digitalWrite(PIN_LED_ARMED, LOW);
    digitalWrite(PIN_BUZZER, LOW); 
    
    digitalWrite(PIN_LED_ZONA1, LOW);
    digitalWrite(PIN_LED_ZONA2, LOW);
    digitalWrite(PIN_LED_ZONA3, LOW);
    digitalWrite(PIN_LED_ZONA4, LOW);
}

void Armado::update() {
    if (!sistemaArmado) return;

    if (!alarmaDisparada) {
        monitorearZonas();
    } else {
        gestionarTiempos();
    }
}

void Armado::monitorearZonas() {
    uint8_t pinesZonas[4] = {PIN_ZONA1, PIN_ZONA2, PIN_ZONA3, PIN_ZONA4};
    
    for (int i = 0; i < 4; i++) {
        if (digitalRead(pinesZonas[i]) == HIGH) { 
            zonaViolada = i + 1;
            alarmaDisparada = true;
            alarmaFisicaActiva = true;
            tiempoDisparo = millis();
            
            if (digitalRead(PIN_DIP_SILENCIOSA) == LOW) { 
                digitalWrite(PIN_BUZZER, HIGH);
                sirenaActiva = true;
            }
            break; 
        }
    }
}

void Armado::gestionarTiempos() {
    unsigned long tiempoActual = millis();
    unsigned long tiempoTranscurrido = tiempoActual - tiempoDisparo;

    if (!advertenciaESPEnviada && tiempoTranscurrido >= TIEMPO_ESPERA_ESP) {
        comunicarESP32("ALERTA: INTRUSION NO ATENDIDA EN ZONA " + String(zonaViolada));
        advertenciaESPEnviada = true;
    }

    if (alarmaFisicaActiva) {
        unsigned long duracionAlarma = map(analogRead(PIN_POT2), 0, 1023, 5000, 120000); 
        
        if (tiempoTranscurrido >= duracionAlarma) {
            digitalWrite(PIN_BUZZER, LOW); 
            sirenaActiva = false;
            alarmaFisicaActiva = false; 
        } else {
            if (tiempoActual - ultimoParpadeo >= INTERVALO_PARPADEO) {
                ultimoParpadeo = tiempoActual;
                estadoParpadeoLED = !estadoParpadeoLED;
                parpadearLEDZona();
            }
        }
    }
}

void Armado::parpadearLEDZona() {
    uint8_t pinLedActual;
    switch(zonaViolada) {
        case 1: pinLedActual = PIN_LED_ZONA1; break;
        case 2: pinLedActual = PIN_LED_ZONA2; break;
        case 3: pinLedActual = PIN_LED_ZONA3; break;
        case 4: pinLedActual = PIN_LED_ZONA4; break;
        default: return;
    }
    digitalWrite(pinLedActual, estadoParpadeoLED ? HIGH : LOW);
}

void Armado::triggerPanico() {
    if (!sistemaArmado) activarArmado(); 
    
    zonaViolada = 99; 
    alarmaDisparada = true;
    alarmaFisicaActiva = true;
    tiempoDisparo = millis();
    
    digitalWrite(PIN_BUZZER, HIGH); 
    sirenaActiva = true;
    
    comunicarESP32("ALERTA CRITICA: BOTON DE PANICO ACCIONADO");
    advertenciaESPEnviada = true; 
}

void Armado::comunicarESP32(String mensaje) {
    Serial.println(mensaje); 
}

bool Armado::isAlarmaDisparada() {
    return alarmaDisparada;
}

uint8_t Armado::getZonaViolada() {
    return zonaViolada;
}

bool Armado::isSistemaArmado() {
    return sistemaArmado;
}