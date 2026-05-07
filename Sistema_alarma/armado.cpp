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
    
    digitalWrite(PIN_LED_ARMED, HIGH); // LED 2 Steady when armed [cite: 67]
}

void Armado::desactivarArmado() {
    sistemaArmado = false;
    alarmaDisparada = false;
    alarmaFisicaActiva = false;
    sirenaActiva = false;
    
    digitalWrite(PIN_LED_ARMED, LOW);
    digitalWrite(PIN_BUZZER, LOW); // Siren stops immediately [cite: 91]
    
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
    // Switch 1-4: Intrusion sensor zones [cite: 42]
    uint8_t pinesZonas[4] = {PIN_ZONA1, PIN_ZONA2, PIN_ZONA3, PIN_ZONA4};
    
    for (int i = 0; i < 4; i++) {
        if (digitalRead(pinesZonas[i]) == HIGH) { 
            zonaViolada = i + 1;
            alarmaDisparada = true;
            alarmaFisicaActiva = true;
            tiempoDisparo = millis();
            
            // Switch 6: Silent alarm (no siren) vs. audible [cite: 43]
            if (digitalRead(PIN_DIP_SILENCIOSA) == LOW) { // Asumiendo LOW = Audible
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

    // HILO 1: Red y Notificación (Independiente del Hardware)
    if (!advertenciaESPEnviada && tiempoTranscurrido >= TIEMPO_ESPERA_ESP) {
        comunicarESP32("ALERTA: INTRUSION NO ATENDIDA EN ZONA " + String(zonaViolada));
        advertenciaESPEnviada = true;
    }

    // HILO 2: Actuadores Físicos y Control Local
    if (alarmaFisicaActiva) {
        // Pot2: Adjust alarm duration/siren length [cite: 47]
        // Se mapea la lectura de 0-1023 a un rango de tiempo en ms (Ej: 5 a 120 seg)
        unsigned long duracionAlarma = map(analogRead(PIN_POT2), 0, 1023, 5000, 120000); 
        
        if (tiempoTranscurrido >= duracionAlarma) {
            digitalWrite(PIN_BUZZER, LOW); 
            sirenaActiva = false;
            alarmaFisicaActiva = false; // Finaliza la alerta física, pero alarmaDisparada sigue TRUE
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
    // Button1: Panic button (instant alarm) [cite: 49]
    if (!sistemaArmado) activarArmado(); 
    
    zonaViolada = 99; // 99 indica Pánico en el sistema
    alarmaDisparada = true;
    alarmaFisicaActiva = true;
    tiempoDisparo = millis();
    
    digitalWrite(PIN_BUZZER, HIGH); // El pánico es estrictamente audible
    sirenaActiva = true;
    
    comunicarESP32("ALERTA CRITICA: BOTON DE PANICO ACCIONADO");
    advertenciaESPEnviada = true; // Ignora los 30s de espera
}

void Armado::comunicarESP32(String mensaje) {
    // Ajustar según el puerto que asignen para la comunicación (Serial, Serial1, etc.)
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