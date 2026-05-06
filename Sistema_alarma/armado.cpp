#include "armado.h"

EstadoArmado::EstadoArmado() 
  : tiempoInicio(0), sistemaArmado(false)
{
}

void EstadoArmado::enter() {
  Serial.println("[ARMADO] Estado iniciado");
  tiempoInicio = millis();
  sistemaArmado = true;
}

void EstadoArmado::update() {
  // Lógica de actualización para estado armado
}

void EstadoArmado::exit() {
  Serial.println("[ARMADO] Estado finalizado");
  sistemaArmado = false;
}
