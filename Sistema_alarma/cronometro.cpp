#include "cronometro.h"

Cronometro::Cronometro() 
  : tiempoInicio(0), tiempoTranscurrido(0), enMarcha(false), duracionAlarma(30)
{
}

void Cronometro::iniciar() {
  tiempoInicio = millis();
  enMarcha = true;
}

void Cronometro::detener() {
  enMarcha = false;
}

void Cronometro::reiniciar() {
  tiempoInicio = 0;
  tiempoTranscurrido = 0;
  enMarcha = false;
}

unsigned long Cronometro::obtenerTiempoTranscurrido() {
  if (enMarcha) {
    tiempoTranscurrido = millis() - tiempoInicio;
  }
  return tiempoTranscurrido;
}

void Cronometro::establecerDuracionAlarma(unsigned int segundos) {
  duracionAlarma = segundos;
}
