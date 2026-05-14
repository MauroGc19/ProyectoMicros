#include "armado.h"

const unsigned long TIEMPO_ESPERA_ESP = 30000; // 30 segundos inamovibles para red
const unsigned long INTERVALO_PARPADEO = 150;  // Parpadeo rápido en milisegundos

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
    
    // LED2: Armed status (steady when armed) 
    digitalWrite(LED_2, HIGH); 
}

void Armado::desactivarArmado() {
    sistemaArmado = false;
    alarmaDisparada = false;
    alarmaFisicaActiva = false;
    sirenaActiva = false;
    
    digitalWrite(LED_2, LOW);
    digitalWrite(RELAY_PIN, LOW); // Relé: Main alarm output - activates siren/strobe light 
    
    // Apagar LEDs de zona (LED3-6: Zone status) 
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, LOW);
    digitalWrite(LED_5, LOW);
    digitalWrite(LED_6, LOW);
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
    // Switch 1-4: Intrusion sensor zones [cite: 42]
    uint8_t pinesZonas[4] = {SWITCH_1, SWITCH_2, SWITCH_3, SWITCH_4};
    
    for (int i = 0; i < 4; i++) {
        if (digitalRead(pinesZonas[i]) == HIGH) { 
            zonaViolada = i + 1;
            alarmaDisparada = true;
            alarmaFisicaActiva = true;
            tiempoDisparo = millis();
            
            // Switch 6: Silent alarm (no siren) vs. audible [cite: 43]
            if (digitalRead(SWITCH_6) == LOW) { // Asumiendo LOW = Audible
                digitalWrite(RELAY_PIN, HIGH);
                sirenaActiva = true;
            }
            break; 
        }
    }
}

void Armado::gestionarTiempos() {
    unsigned long tiempoActual = millis();
    unsigned long tiempoTranscurrido = tiempoActual - tiempoDisparo;

    // HILO 1: Red y Notificación (Independiente del Hardware)
    if (!advertenciaESPEnviada && tiempoTranscurrido >= TIEMPO_ESPERA_ESP) {
        comunicarESP32("ALERTA: INTRUSION NO ATENDIDA EN ZONA " + String(zonaViolada));
        advertenciaESPEnviada = true;
    }

    // HILO 2: Actuadores Físicos y Control Local
    if (alarmaFisicaActiva) {
        // Port2: Adjust alarm duration/siren length [cite: 47]
        unsigned long duracionAlarma = map(analogRead(POT_2), 0, 1023, 5000, 120000); 
        
        if (tiempoTranscurrido >= duracionAlarma) {
            digitalWrite(RELAY_PIN, LOW); 
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
    // LED3-6: Zone status (show which zones are active/violated) 
    switch(zonaViolada) {
        case 1: pinLedActual = LED_3; break;
        case 2: pinLedActual = LED_4; break;
        case 3: pinLedActual = LED_5; break;
        case 4: pinLedActual = LED_6; break;
        default: return;
    }
    digitalWrite(pinLedActual, estadoParpadeoLED ? HIGH : LOW);
}

void Armado::triggerPanico() {
    // Button1: Panic button (instant alarm) [cite: 49]
    if (!sistemaArmado) activarArmado(); 
    
    zonaViolada = 99; // 99 indica Pánico en el sistema
    alarmaDisparada = true;
    alarmaFisicaActiva = true;
    tiempoDisparo = millis();
    
    digitalWrite(RELAY_PIN, HIGH); // El pánico es estrictamente audible
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