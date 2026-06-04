#include "EstadoArmado.h"
#include "EstadoDesarmado.h" // solo para crear nueva instancia al desarmar
#include <Arduino.h>

// Referencias externas
extern LiquidCrystal lcd;
extern Keypad teclado;
extern void cambiarEstado(void* nuevoEstado);

// Inicializar bandera estática
bool EstadoArmado::alarmaMemoria = false;

EstadoArmado::EstadoArmado()
  : pinIngresado(""), ultimaTeclaMillis(0), preAlarmaActiva(false), zonaActiva(0),
    entradaSegundos(0), tiempoRestanteEntrada(0), lastEntradaTickMillis(0), preAlarmaStartMillis(0),
    sirenaActiva(false), currentDigit(0), lastMultiplexMicros(0), lastLEDBlinkMillis(0), ledBlinkState(false),
    mensajeIncorrectoUntilMillis(0), mensajeIncorrecto(""), led8UntilMillis(0)
{
}

bool EstadoArmado::isLed8Active() {
  return millis() < led8UntilMillis;
}

bool EstadoArmado::isSirenaActive() {
  return sirenaActiva;
}

void EstadoArmado::begin() {
  // Configurar pines
  pinMode(DISPLAY_1_EN, OUTPUT);
  pinMode(DISPLAY_2_EN, OUTPUT);
  pinMode(DATA_BIT_0, OUTPUT);
  pinMode(DATA_BIT_1, OUTPUT);
  pinMode(DATA_BIT_2, OUTPUT);
  pinMode(DATA_BIT_3, OUTPUT);

  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
  pinMode(LED_6, OUTPUT);
  pinMode(LED_7, OUTPUT);
  pinMode(LED_8, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Aseguramos que la memoria de alarma deje de parpadear cuando el sistema se Arme nuevamente
  alarmaMemoria = false;
  digitalWrite(LED_7, LOW);
  digitalWrite(LED_8, LOW);

  // Inicializa pantalla
  lcd.begin(16,2);
  mostrarLCD();
}

void EstadoArmado::enter() { begin(); }
void EstadoArmado::update() { actualizar(); }

void EstadoArmado::exit() {
  // Apagar displays
  digitalWrite(DISPLAY_1_EN, LOW);
  digitalWrite(DISPLAY_2_EN, LOW);
}

void EstadoArmado::actualizar() {
  // Lecturas no bloqueantes
  monitorearZonas();
  leerTeclado();
  manejarLEDs();

  // Actualiza pre-alarma (conteo)
  if (preAlarmaActiva) {
    unsigned long now = millis();
    if (now - lastEntradaTickMillis >= 1000) {
      lastEntradaTickMillis += 1000;
      if (tiempoRestanteEntrada > 0) tiempoRestanteEntrada--;
      if (tiempoRestanteEntrada == 0) {
        // Tiempo de entrada expiró: activar sirena si SWITCH_5 activado
        activarSirena();
      }
    }
  }

  // Actualizaciones visuales
  mostrarLCD();
  mostrarDisplays();
}

// Monitorea las zonas (SWITCH_1..4). Si detecta cambio a activo inicia pre-alarma.
void EstadoArmado::monitorearZonas() {
  // Usamos digitalRead sobre pines analógicos (A2..A5 funcionan como digitales)
  if (!preAlarmaActiva) {
    if (digitalRead(SWITCH_1) == HIGH) { activarPreAlarma(1); }
    else if (digitalRead(SWITCH_2) == HIGH) { activarPreAlarma(2); }
    else if (digitalRead(SWITCH_3) == HIGH) { activarPreAlarma(3); }
    else if (digitalRead(SWITCH_4) == HIGH) { activarPreAlarma(4); }
  }
}

void EstadoArmado::activarPreAlarma(uint8_t zona) {
  preAlarmaActiva = true;
  zonaActiva = zona;
  entradaSegundos = leerPot1Segundos();
  if (entradaSegundos == 0) entradaSegundos = 1; // mínimo 1 seg
  tiempoRestanteEntrada = entradaSegundos;
  lastEntradaTickMillis = millis();
  preAlarmaStartMillis = millis();
  pinIngresado = "";
  ultimaTeclaMillis = millis();
  // Encender LED correspondiente (manejo visual en manejarLEDs)
}

void EstadoArmado::detenerPreAlarma() {
  preAlarmaActiva = false;
  zonaActiva = 0;
  tiempoRestanteEntrada = 0;
}

// Activa la sirena si SWITCH_5 == ACTIVADO
void EstadoArmado::activarSirena() {
  // Marcar memoria de alarma
  alarmaMemoria = true;

  // Si SWITCH_5 está activado, activar relay
  if (digitalRead(SWITCH_5) == HIGH) {
    digitalWrite(RELAY_PIN, HIGH);
    sirenaActiva = true;

    // Notificar al ESP32
    if (digitalRead(SWITCH_7) == HIGH) {
      Serial1.println("INTRUSION_PRUEBA");
    } else {
      Serial1.println("INTRUSION_REAL");
    }
  } else {
    // Alarma silenciosa: no activar relay, pero marcar memoria
    sirenaActiva = false;
  }

  // Dejar pre-alarma desactivada pero conservar zonaActiva para indicación
  preAlarmaActiva = false;

  // Iniciar parpadeo de LED_7 (memoria)

  // Encender LED_8 por la duración configurada en POT_2
  unsigned int dur = 0;
  int raw = analogRead(POT_2);
  dur = constrain(map(raw, 0, 1023, 0, 99), 0, 99);
  if (dur == 0) dur = 1; // al menos 1 segundo
  led8UntilMillis = millis() + ((unsigned long)dur * 1000UL);
}

// Lectura del teclado 4x4 no bloqueante
void EstadoArmado::leerTeclado() {
  char k = teclado.getKey();
  if (k) {
    // Aceptar números y teclas especiales
    if (k >= '0' && k <= '9') {
      if (pinIngresado.length() < 6) pinIngresado += k; // permitir más intentos
      ultimaTeclaMillis = millis();
    } else if (k == '#') {
      // Clear input
      pinIngresado = "";
      ultimaTeclaMillis = millis();
    } else if (k == '*') {
      // Submit PIN
      if (validarPIN()) {
        // Si la sirena está activa o hubo pre-alarma, apagar y desarmar
        bool mantenerMemoria = alarmaMemoria || sirenaActiva; 
        resetearSistema(mantenerMemoria);
        cambiarEstado((void*)1);
        return;
      } else {
        // PIN incorrecto
        mensajeIncorrecto = "PIN Incorrecto";
        mensajeIncorrectoUntilMillis = millis() + 1500;
        pinIngresado = "";
      }
    }
  }

  // Timeout para limpieza automática
  if (pinIngresado.length() > 0 && (millis() - ultimaTeclaMillis) > PIN_TIMEOUT_MS) {
    pinIngresado = "";
  }
}

bool EstadoArmado::validarPIN() {
  return pinIngresado == PIN_CORRECTO;
}

// Muestra en LCD según estado
void EstadoArmado::mostrarLCD() {
  static unsigned long lastLCD = 0;
  if (millis() - lastLCD < 200) return; // refresco controlado
  lastLCD = millis();

  lcd.clear();
  if (preAlarmaActiva) {
    lcd.setCursor(0,0);
    lcd.print("ALARMA!");
    lcd.setCursor(0,1);
    lcd.print("Zona ");
    lcd.print(zonaActiva);
    // Mostrar tiempo restante
    lcd.setCursor(9,1);
    if (tiempoRestanteEntrada < 10) lcd.print('0');
    lcd.print(tiempoRestanteEntrada);
    lcd.print('s');
    return;
  }

  // Estado armado normal
  lcd.setCursor(0,0);
  lcd.print("ALARMA ARMADA");
  lcd.setCursor(0,1);
  // Si hay mensaje de PIN incorrecto mostrarlo temporalmente
  if (mensajeIncorrectoUntilMillis > millis()) {
    lcd.print(mensajeIncorrecto);
    // completar resto de la línea
    for (uint8_t i = mensajeIncorrecto.length(); i < 16; i++) lcd.print(' ');
  } else {
    lcd.print("Monitoreando   ");
  }

  // Si hay entrada de PIN mostrar asteriscos
  if (pinIngresado.length() > 0) {
    lcd.setCursor(11,1);
    for (uint8_t i=0;i<4;i++) {
      if (i < pinIngresado.length()) lcd.print('*'); else lcd.print(' ');
    }
  }
}

// Muestra en displays multiplexados: si pre-alarma mostrar número de zona
void EstadoArmado::mostrarDisplays() {
  unsigned long now = micros();
  if (now - lastMultiplexMicros < MULTIPLEX_INTERVAL_US) return;
  lastMultiplexMicros = now;

  uint8_t decenas = 0;
  uint8_t unidades = 0;

  if (preAlarmaActiva) {
    uint8_t valor = zonaActiva % 10;
    decenas = 0;
    unidades = valor;
  } else {
    // Mostrar 00 cuando no hay pre-alarma
    decenas = 0; unidades = 0;
  }

  if (currentDigit == 0) {
    digitalWrite(DISPLAY_2_EN, LOW);
    mostrarNumeroBCD(0, decenas);
    digitalWrite(DISPLAY_1_EN, HIGH);
    currentDigit = 1;
  } else {
    digitalWrite(DISPLAY_1_EN, LOW);
    mostrarNumeroBCD(1, unidades);
    digitalWrite(DISPLAY_2_EN, HIGH);
    currentDigit = 0;
  }
}

void EstadoArmado::mostrarNumeroBCD(uint8_t /*displayIndex*/, uint8_t digito) {
  digitalWrite(DATA_BIT_0, (digito & 0x01) ? HIGH : LOW);
  digitalWrite(DATA_BIT_1, (digito & 0x02) ? HIGH : LOW);
  digitalWrite(DATA_BIT_2, (digito & 0x04) ? HIGH : LOW);
  digitalWrite(DATA_BIT_3, (digito & 0x08) ? HIGH : LOW);
}

// Manejo de LEDs: zona y memoria (LED_7)
void EstadoArmado::manejarLEDs() {
  unsigned long now = millis();

  // LED correspondiente a zona activa (si pre-alarma)
  if (preAlarmaActiva) {
    // parpadeo rápido del LED de zona
    if (now - lastLEDBlinkMillis >= LED_FAST_BLINK_MS) {
      lastLEDBlinkMillis = now;
      ledBlinkState = !ledBlinkState;
    }
    bool on = ledBlinkState;
    digitalWrite(LED_3, (zonaActiva==1 && on) ? HIGH : LOW);
    digitalWrite(LED_4, (zonaActiva==2 && on) ? HIGH : LOW);
    digitalWrite(LED_5, (zonaActiva==3 && on) ? HIGH : LOW);
    digitalWrite(LED_6, (zonaActiva==4 && on) ? HIGH : LOW);
  } else {
    // Apagar LEDs de zona si no hay pre-alarma y sirena no activa
    digitalWrite(LED_3, LOW);
    digitalWrite(LED_4, LOW);
    digitalWrite(LED_5, LOW);
    digitalWrite(LED_6, LOW);
  }

  // LED_7: memoria de alarma debe parpadear si alarmaMemoria == true
  if (alarmaMemoria) {
    if (now - lastLEDBlinkMillis >= LED_BLINK_MS) {
      lastLEDBlinkMillis = now;
      ledBlinkState = !ledBlinkState;
      digitalWrite(LED_7, ledBlinkState ? HIGH : LOW);
    }
  } else {
    // Si no hay memoria, LED_7 indica sirena activa si corresponde
    if (sirenaActiva) digitalWrite(LED_7, HIGH);
    else digitalWrite(LED_7, LOW);
  }
}

// Resetea sistema: apagar relay y LEDs. mantenerMemoria indica si LED_7 debe seguir parpadeando
void EstadoArmado::resetearSistema(bool mantenerMemoria) {
  digitalWrite(RELAY_PIN, LOW);
  sirenaActiva = false;
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
  digitalWrite(LED_5, LOW);
  digitalWrite(LED_6, LOW);
  alarmaMemoria = mantenerMemoria;
}

// Lee POT_1 y lo mapea a segundos 0..99 (igual que EstadoDesarmado)
unsigned int EstadoArmado::leerPot1Segundos() {
  int raw = analogRead(POT_1);
  int mapped = constrain(map(raw, 0, 1023, 0, 99), 0, 99);
  return (unsigned int)mapped;
}
