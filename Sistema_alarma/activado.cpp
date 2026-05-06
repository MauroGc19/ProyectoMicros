#include "activado.h"

EstadoActivado::EstadoActivado() 
  : tiempoInicio(0), alarmaActiva(false)
{
}

void EstadoActivado::enter() {
  Serial.println("[ACTIVADO] Alarma disparada");
  tiempoInicio = millis();
  alarmaActiva = true;
}

void EstadoActivado::update() {
  // Lógica de actualización para estado de alarma
}

void EstadoActivado::exit() {
  Serial.println("[ACTIVADO] Alarma finalizada");
  alarmaActiva = false;
}
