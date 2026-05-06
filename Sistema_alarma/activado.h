#ifndef ACTIVADO_H
#define ACTIVADO_H

#include <Arduino.h>
#include "pins.h"

class EstadoActivado {
private:
  // Variables de control
  unsigned long tiempoInicio;
  bool alarmaActiva;
  
public:
  EstadoActivado();
  void enter();
  void update();
  void exit();
};

#endif // ACTIVADO_H
