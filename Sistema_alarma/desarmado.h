#ifndef DESARMADO_H
#define DESARMADO_H

#include <Arduino.h>
#include "pins.h"

class EstadoDesarmado {
private:
  // PIN de seguridad y configuración
  String pinSeguridad;           // PIN correcto (4 dígitos)
  String pinIngresado;           // PIN que el usuario está ingresando
  unsigned long ultimaTeclaTime; // Tiempo de la última tecla presionada
  const unsigned long TIMEOUT_PIN = 5000; // 5 segundos timeout
  
  // Parámetros del sistema
  unsigned int tiempoCronometro; // Almacena tiempo del cronómetro en segundos
  unsigned int tiempoAlarma;     // Almacena duración de la alarma en segundos
  
  // Estados interno
  bool pinValido;                // Indica si el PIN ingresado es correcto
  bool sistemaConfigurado;       // Indica si los parámetros se han configurado
  
public:
  // Constructor
  EstadoDesarmado();
  
  // Métodos de ciclo de vida del estado
  void enter();                  // Se llama al entrar al estado
  void update();                 // Se llama cada ciclo para actualizar el estado
  void exit();                   // Se llama al salir del estado
  
  // Métodos de entrada
  void procesarTecla(char tecla);     // Procesa tecla del teclado matricial
  void leerPotenciometros();          // Lee y actualiza valores de potenciómetros
  
  // Métodos de validación
  bool verificarPin();                // Verifica si el PIN ingresado es correcto
  void actualizarTiempoCronometro(int valorAnalogico); // Mapea POT1 a tiempo
  void actualizarTiempoAlarma(int valorAnalogico);     // Mapea POT2 a tiempo
  
  // Getters para otros módulos
  unsigned int obtenerTiempoCronometro() const;
  unsigned int obtenerTiempoAlarma() const;
  bool estaConfigurado() const;
  bool debeTransicionarArmado() const;
  
  // Métodos de interfaz
  void mostrarMensajeEnLCD();         // Muestra estado actual en LCD
  void actualizarLED();               // Controla LED 1 como indicador
  
  // Reset
  void reiniciar();                   // Reinicia el estado
};

#endif // DESARMADO_H
