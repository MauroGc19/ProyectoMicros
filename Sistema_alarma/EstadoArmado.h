#ifndef ESTADOARMADO_H
#define ESTADOARMADO_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include "pins.h"

// Referencias externas proporcionadas por el sketch principal
extern LiquidCrystal lcd;
extern Keypad teclado;
extern void cambiarEstado(void* nuevoEstado);

class EstadoArmado {
public:
  EstadoArmado();
  // Inicializa pines y variables
  void begin();
  // Ciclo principal: no bloqueante
  void actualizar();

  // Métodos solicitados
  void monitorearZonas();
  void activarPreAlarma(uint8_t zona);
  void activarSirena();
  void leerTeclado();
  bool validarPIN();
  void mostrarLCD();
  void mostrarDisplays();
  void manejarLEDs();
  void resetearSistema(bool mantenerMemoria);
  // Indica si el LED_8 está activado por temporizador interno
  bool isLed8Active();

  // Compatibilidad simple con sketch
  void enter();
  void update();
  void exit();

private:
  // PIN de desactivación del sistema en estado armado
  const String PIN_CORRECTO = "1234";
  String pinIngresado;
  unsigned long ultimaTeclaMillis;
  const unsigned long PIN_TIMEOUT_MS = 5000;

  // Variables para pre-alarma
  bool preAlarmaActiva;
  uint8_t zonaActiva; // 1..4
  unsigned int entradaSegundos; // tiempo de entrada (desde POT_1)
  unsigned int tiempoRestanteEntrada;
  unsigned long lastEntradaTickMillis;
  unsigned long preAlarmaStartMillis;

  // Sirena
  bool sirenaActiva;

  // Displays multiplex
  uint8_t currentDigit;
  unsigned long lastMultiplexMicros;
  const unsigned long MULTIPLEX_INTERVAL_US = 2000; // 2ms por dígito

  // LEDs
  unsigned long lastLEDBlinkMillis;
  const unsigned long LED_BLINK_MS = 500;
  const unsigned long LED_FAST_BLINK_MS = 150;
  bool ledBlinkState;

  // Mensaje temporal de PIN incorrecto
  unsigned long mensajeIncorrectoUntilMillis;
  String mensajeIncorrecto;

  // Memoria de alarma: persiste hasta que el sistema se arme nuevamente
  static bool alarmaMemoria;

  // Temporizador interno para LED_8 cuando suena la alarma
  unsigned long led8UntilMillis;

  // Helpers
  unsigned int leerPot1Segundos();
  void mostrarNumeroBCD(uint8_t displayIndex, uint8_t digito);
  void detenerPreAlarma();
};

#endif // ESTADOARMADO_H
