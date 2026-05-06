#ifndef CRONOMETRO_H
#define CRONOMETRO_H

#include <Arduino.h>

class Cronometro {
private:
  unsigned long tiempoInicio;
  unsigned long tiempoTranscurrido;
  bool enMarcha;
  unsigned int duracionAlarma;
  
public:
  Cronometro();
  void iniciar();
  void detener();
  void reiniciar();
  unsigned long obtenerTiempoTranscurrido();
  void establecerDuracionAlarma(unsigned int segundos);
};

#endif // CRONOMETRO_H
