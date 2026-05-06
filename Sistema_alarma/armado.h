#ifndef ARMADO_H
#define ARMADO_H

#include <Arduino.h>
#include "pins.h"

class EstadoArmado {
private:
  // Variables de control
  unsigned long tiempoInicio;
  bool sistemaArmado;
  
public:
  EstadoArmado();
  void enter();
  void update();
  void exit();
};

#endif // ARMADO_H
