#include "desarmado.h"
#include <LiquidCrystal.h>

// Instancia global del LCD (para uso en esta clase)
// Se asume que será inicializada desde el programa principal
extern LiquidCrystal lcd;

// Constructor
EstadoDesarmado::EstadoDesarmado() 
  : pinSeguridad("1234"),        // PIN por defecto
    pinIngresado(""),
    ultimaTeclaTime(0),
    tiempoCronometro(30),         // Valor por defecto: 30 segundos
    tiempoAlarma(30),             // Valor por defecto: 30 segundos
    pinValido(false),
    sistemaConfigurado(false)
{
}

// Se llama al entrar al estado
void EstadoDesarmado::enter() {
  // Limpiar entrada anterior
  pinIngresado = "";
  pinValido = false;
  sistemaConfigurado = false;
  
  // Configurar LED 1 como salida
  pinMode(LED_1, OUTPUT);
  
  // Mostrar mensaje inicial
  mostrarMensajeEnLCD();
  actualizarLED();
  
  Serial.println("[DESARMADO] Estado iniciado - Esperando PIN de seguridad");
}

// Se llama cada ciclo para actualizar el estado
void EstadoDesarmado::update() {
  // Leer valores de potenciómetros en tiempo real
  leerPotenciometros();
  
  // Verificar timeout del PIN
  if (pinIngresado.length() > 0 && millis() - ultimaTeclaTime > TIMEOUT_PIN) {
    Serial.println("[DESARMADO] Timeout - PIN rechazado");
    pinIngresado = "";
    pinValido = false;
    mostrarMensajeEnLCD();
  }
  
  // Actualizar LED y LCD
  actualizarLED();
}

// Se llama al salir del estado
void EstadoDesarmado::exit() {
  // Apagar LED 1 al salir
  digitalWrite(LED_1, LOW);
  
  Serial.println("[DESARMADO] Estado finalizado - Transición a siguiente estado");
}

// Procesa una tecla del teclado matricial
void EstadoDesarmado::procesarTecla(char tecla) {
  ultimaTeclaTime = millis();
  
  // Si la tecla es '#' (fila 4, columna 1), validar el PIN
  if (tecla == '#') {
    if (verificarPin()) {
      pinValido = true;
      sistemaConfigurado = true;
      Serial.println("[DESARMADO] PIN correcto - Sistema configurado");
      Serial.print("Tiempo cronómetro: ");
      Serial.print(tiempoCronometro);
      Serial.println(" segundos");
      Serial.print("Tiempo alarma: ");
      Serial.print(tiempoAlarma);
      Serial.println(" segundos");
      mostrarMensajeEnLCD();
    } else {
      Serial.println("[DESARMADO] PIN incorrecto");
      pinIngresado = "";
      mostrarMensajeEnLCD();
    }
  }
  // Si es '*', limpiar el PIN actual
  else if (tecla == '*') {
    pinIngresado = "";
    Serial.println("[DESARMADO] PIN limpiado");
    mostrarMensajeEnLCD();
  }
  // Agregar dígito al PIN si no hemos alcanzado 4 caracteres
  else if (pinIngresado.length() < 4 && tecla >= '0' && tecla <= '9') {
    pinIngresado += tecla;
    Serial.print("[DESARMADO] Tecla ingresada: ");
    Serial.println(tecla);
    mostrarMensajeEnLCD();
  }
  // Ignorar otras teclas (A, B, C, D)
}

// Lee y procesa los potenciómetros
void EstadoDesarmado::leerPotenciometros() {
  // Leer potenciómetro 1 (tiempo del cronómetro)
  int valor_pot1 = analogRead(POT_1); // 0-1023
  actualizarTiempoCronometro(valor_pot1);
  
  // Leer potenciómetro 2 (tiempo de la alarma)
  int valor_pot2 = analogRead(POT_2); // 0-1023
  actualizarTiempoAlarma(valor_pot2);
}

// Mapea el valor del POT1 al tiempo del cronómetro
void EstadoDesarmado::actualizarTiempoCronometro(int valorAnalogico) {
  // Mapeo: 0 -> armado inmediato, 1023 -> 30 segundos
  // Mapeo lineal: valor_segundos = (valor_analogo / 1023) * 30
  
  if (valorAnalogico < 50) {
    // Zona muerta: menos de 50 significa armado inmediato (0 segundos)
    tiempoCronometro = 0;
  } else {
    // Mapeo: 50-1023 -> 1-30 segundos
    tiempoCronometro = map(valorAnalogico, 50, 1023, 1, 30);
  }
}

// Mapea el valor del POT2 al tiempo de la alarma
void EstadoDesarmado::actualizarTiempoAlarma(int valorAnalogico) {
  // Mapeo: 0 -> 5 segundos, 1023 -> 120 segundos
  // Rango: 5 a 120 segundos
  
  tiempoAlarma = map(valorAnalogico, 0, 1023, 5, 120);
}

// Verifica si el PIN ingresado es correcto
bool EstadoDesarmado::verificarPin() {
  return (pinIngresado == pinSeguridad);
}

// Getters
unsigned int EstadoDesarmado::obtenerTiempoCronometro() const {
  return tiempoCronometro;
}

unsigned int EstadoDesarmado::obtenerTiempoAlarma() const {
  return tiempoAlarma;
}

bool EstadoDesarmado::estaConfigurado() const {
  return sistemaConfigurado;
}

bool EstadoDesarmado::debeTransicionarArmado() const {
  // Transiciona a armado si el PIN fue validado correctamente
  return (pinValido && sistemaConfigurado);
}

// Muestra mensaje en LCD
void EstadoDesarmado::mostrarMensajeEnLCD() {
  lcd.clear();
  
  if (!pinValido) {
    // Mostrar estado de entrada de PIN
    lcd.setCursor(0, 0);
    lcd.print("Alarma Desarmada");
    
    lcd.setCursor(0, 1);
    if (pinIngresado.length() == 0) {
      lcd.print("PIN: ____");
    } else {
      lcd.print("PIN: ");
      for (int i = 0; i < pinIngresado.length(); i++) {
        lcd.print("*");
      }
      // Mostrar espacios en blanco para dígitos faltantes
      for (int i = pinIngresado.length(); i < 4; i++) {
        lcd.print("_");
      }
    }
  } else {
    // Mostrar parámetros configurados
    lcd.setCursor(0, 0);
    lcd.print("Configurado OK");
    
    lcd.setCursor(0, 1);
    lcd.print("T.Arm:");
    lcd.print(tiempoCronometro);
    lcd.print("s Ala:");
    lcd.print(tiempoAlarma);
    lcd.print("s");
  }
}

// Controla el LED 1 como indicador
void EstadoDesarmado::actualizarLED() {
  // LED 1 siempre encendido en estado desarmado
  digitalWrite(LED_1, HIGH);
}

// Reinicia el estado
void EstadoDesarmado::reiniciar() {
  pinIngresado = "";
  pinValido = false;
  sistemaConfigurado = false;
  ultimaTeclaTime = 0;
  tiempoCronometro = 30;
  tiempoAlarma = 30;
}
