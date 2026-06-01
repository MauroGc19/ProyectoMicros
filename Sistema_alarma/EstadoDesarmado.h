// EstadoDesarmado.h
#ifndef ESTADODESARMADO_H
#define ESTADODESARMADO_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include "pins.h"

// Declaraciones externas (definidas en el sketch principal)
extern LiquidCrystal lcd; // instancia global del LCD creada en Sistema_alarma.ino
extern Keypad teclado;    // instancia global del teclado creada en Sistema_alarma.ino
// Función que realiza el cambio de estado. El prototipo se deja genérico
// para que el programa principal implemente: cambiarEstado(new EstadoArmado(...));
extern void cambiarEstado(void* nuevoEstado);

class EstadoDesarmado {
public:
  // Constructor
  EstadoDesarmado();

  // Inicializa pines y variables (invocado desde enter/begin)
  void begin();

  // Ciclo principal: no bloqueante
  void actualizar();

  // Métodos expuestos solicitados
  void leerPotenciometros();
  void leerTeclado();
  void mostrarLCD();
  void mostrarDisplays();
  void iniciarCuentaRegresiva();
  void mostrarNumeroBCD(uint8_t displayIndex, uint8_t digito);
  bool validarPIN();

  // Métodos de compatibilidad con el sketch existente
  void enter();
  void update();
  void exit();
  void procesarTecla(char tecla);

  // Getters usados por el sketch
  unsigned int obtenerTiempoCronometro() const;
  unsigned int obtenerTiempoAlarma() const;
  bool estaConfigurado() const;
  bool debeTransicionarArmado() const;
  void reiniciar();

private:
  // PIN de seguridad
  const String PIN_CORRECTO = "1245";
  String pinIngresado;
  unsigned long ultimaTeclaMillis;
  const unsigned long PIN_TIMEOUT_MS = 7000;

  // Potenciómetros mapeados 0-99
  uint8_t pot1Value; // tiempo armado / cuenta regresiva (0-99)
  uint8_t pot2Value; // tiempo alarma (0-99)
  int lastRawPot1;
  int lastRawPot2;

  // Estado de armado/cuenta regresiva
  bool countingDown;
  int countdownSeconds;
  unsigned long lastCountdownTickMillis; // para decrementar cada 1s

  // Multiplexación displays
  uint8_t currentDigit; // 0 = decenas, 1 = unidades
  unsigned long lastMultiplexMillis;
  const unsigned long MULTIPLEX_INTERVAL_US = 1; // 5 ms por dígito

  // Mensajes LCD temporales
  String mensajeTemporal;
  unsigned long mensajeUntilMillis;

  // Señal de transición manejada por update
  bool transitionReady;

  // Timers para refresco de LCD
  unsigned long lastLCDMillis;
  const unsigned long LCD_REFRESH_MS = 200;

  // Helpers privados
  void configurarPines();
  void actualizarCuenta();
  void mostrarAsteriscosLCD();
};

#endif // ESTADODESARMADO_H
