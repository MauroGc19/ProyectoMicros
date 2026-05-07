#include "EstadoDesarmado.h"
#include <Arduino.h>

// Referencias externas (instanciadas en Sistema_alarma.ino)
extern LiquidCrystal lcd;
extern Keypad teclado;
extern void cambiarEstado(void* nuevoEstado);

// Constructor
EstadoDesarmado::EstadoDesarmado()
  : pinIngresado(""), ultimaTeclaMillis(0), pot1Value(0), pot2Value(0), lastRawPot1(-1), lastRawPot2(-1),
    countingDown(false), countdownSeconds(0), lastCountdownTickMillis(0), currentDigit(0), lastMultiplexMillis(0),
    mensajeTemporal(""), mensajeUntilMillis(0), transitionReady(false), lastLCDMillis(0)
{
}

void EstadoDesarmado::begin() {
  configurarPines();
  // Inicializa pantalla
  lcd.begin(16,2);
  mostrarLCD();
}

// Compatibilidad con sketch existente
void EstadoDesarmado::enter() {
  pinIngresado = "";
  ultimaTeclaMillis = millis();
  pot1Value = 0;
  pot2Value = 0;
  lastRawPot1 = lastRawPot2 = -1;
  countingDown = false;
  transitionReady = false;
  begin();
}

void EstadoDesarmado::update() { actualizar(); }

void EstadoDesarmado::exit() {
  // Apaga displays y LED si fuese necesario
  digitalWrite(DISPLAY_1_EN, LOW);
  digitalWrite(DISPLAY_2_EN, LOW);
}

void EstadoDesarmado::procesarTecla(char tecla) {
  // Método de compatibilidad: redirige al lector de teclado
  if (tecla) {
    // Simula entrada de tecla
    if (tecla >= '0' && tecla <= '9') {
      pinIngresado += tecla;
      ultimaTeclaMillis = millis();
      mensajeTemporal = ""; // limpiar mensajes temporales
      mostrarAsteriscosLCD();
      if (pinIngresado.length() >= 4) {
        if (validarPIN()) {
          iniciarCuentaRegresiva();
        } else {
          mensajeTemporal = "PIN Incorrecto";
          mensajeUntilMillis = millis() + 1500;
          pinIngresado = "";
        }
      }
    } else if (tecla == '#') {
      // Clear
      pinIngresado = "";
      mostrarLCD();
    } else if (tecla == '*') {
      // Submit
      if (validarPIN()) iniciarCuentaRegresiva();
      else {
        mensajeTemporal = "PIN Incorrecto";
        mensajeUntilMillis = millis() + 1500;
        pinIngresado = "";
      }
    }
  }
}

// Ciclo principal: llamar desde loop()
void EstadoDesarmado::actualizar() {
  // No bloqueante: leer entradas y actualizar salidas
  leerPotenciometros();
  leerTeclado();
  actualizarCuenta();
  // Actualizaciones visuales
  if (millis() - lastLCDMillis >= LCD_REFRESH_MS) {
    mostrarLCD();
    lastLCDMillis = millis();
  }
  mostrarDisplays();
}

// Leer y mapear potenciómetros a 0-99
void EstadoDesarmado::leerPotenciometros() {
  int raw1 = analogRead(POT_1);
  int raw2 = analogRead(POT_2);
  if (raw1 != lastRawPot1) {
    lastRawPot1 = raw1;
    pot1Value = constrain(map(raw1, 0, 1023, 0, 99), 0, 99);
  }
  if (raw2 != lastRawPot2) {
    lastRawPot2 = raw2;
    pot2Value = constrain(map(raw2, 0, 1023, 0, 99), 0, 99);
  }
}

// Manejo del teclado matricial: usa la instancia global `teclado`
void EstadoDesarmado::leerTeclado() {
  char k = teclado.getKey();
  if (k) {
    procesarTecla(k);
  }
  // Tiempo de espera por timeout de PIN
  if (pinIngresado.length() > 0 && (millis() - ultimaTeclaMillis) > PIN_TIMEOUT_MS) {
    pinIngresado = ""; // limpiar tras timeout
    mostrarLCD();
  }
}

// Mostrar en LCD según estado y mensajes temporales
void EstadoDesarmado::mostrarLCD() {
  lcd.clear();
  // Línea 1: siempre muestra título
  lcd.setCursor(0,0);
  lcd.print("Sistema Desarm");

  // Si hay un mensaje temporal (ej. PIN incorrecto)
  if (mensajeTemporal.length() > 0 && millis() < mensajeUntilMillis) {
    lcd.setCursor(0,1);
    lcd.print(mensajeTemporal);
    return;
  }

  // Si se está ingresando PIN
  if (pinIngresado.length() > 0) {
    lcd.setCursor(0,1);
    lcd.print("Ingrese PIN:");
    lcd.setCursor(11,1);
    mostrarAsteriscosLCD();
    return;
  }

  // Si se está armando (cuenta regresiva)
  if (countingDown) {
    lcd.setCursor(0,1);
    lcd.print("Armando...");
    return;
  }

  // Estado desarmado por defecto: mostrar duración de alarma (POT2)
  lcd.setCursor(0,1);
  lcd.print("Alarma: ");
  if (pot2Value < 10) lcd.print("0");
  lcd.print(pot2Value);
  lcd.print(" seg");
}

// Imprime asteriscos correspondientes al PIN ingresado en la LCD (posición fija)
void EstadoDesarmado::mostrarAsteriscosLCD() {
  // Mostrar hasta 4 asteriscos en la posición final de la línea 2
  uint8_t n = pinIngresado.length();
  lcd.setCursor(11,1);
  for (uint8_t i=0;i<4;i++) {
    if (i < n) lcd.print('*'); else lcd.print(' ');
  }
}

// Mostrar en displays de 7 segmentos (multiplexado)
void EstadoDesarmado::mostrarDisplays() {
  unsigned long now = micros();
  if (now - lastMultiplexMillis < MULTIPLEX_INTERVAL_US) return;
  lastMultiplexMillis = now;

  int valueToShow = countingDown ? countdownSeconds : pot1Value;
  int decenas = (valueToShow / 10) % 10;
  int unidades = valueToShow % 10;

  // Alternar entre displays
  if (currentDigit == 0) {
    // Mostrar decenas en DISPLAY_1
    digitalWrite(DISPLAY_2_EN, LOW);
    mostrarNumeroBCD(0, decenas);
    digitalWrite(DISPLAY_1_EN, HIGH);
    currentDigit = 1;
  } else {
    // Mostrar unidades en DISPLAY_2
    digitalWrite(DISPLAY_1_EN, LOW);
    mostrarNumeroBCD(1, unidades);
    digitalWrite(DISPLAY_2_EN, HIGH);
    currentDigit = 0;
  }
}

// Envía el dígito en BCD a los pines DATA_BIT_x y no hace blocking significativo
void EstadoDesarmado::mostrarNumeroBCD(uint8_t /*displayIndex*/, uint8_t digito) {
  digitalWrite(DATA_BIT_0, (digito & 0x01) ? HIGH : LOW);
  digitalWrite(DATA_BIT_1, (digito & 0x02) ? HIGH : LOW);
  digitalWrite(DATA_BIT_2, (digito & 0x04) ? HIGH : LOW);
  digitalWrite(DATA_BIT_3, (digito & 0x08) ? HIGH : LOW);
}

// Inicia la cuenta regresiva (cuando PIN validado)
void EstadoDesarmado::iniciarCuentaRegresiva() {
  if (pot1Value < 0) pot1Value = 0;
  countdownSeconds = pot1Value;
  countingDown = true;
  lastCountdownTickMillis = millis();
  mensajeTemporal = "Armando...";
  mensajeUntilMillis = millis() + 2000;
}

// Decrementa la cuenta regresiva cada segundo (no bloqueante)
void EstadoDesarmado::actualizarCuenta() {
  if (!countingDown) return;
  unsigned long now = millis();
  if (now - lastCountdownTickMillis >= 1000) {
    lastCountdownTickMillis += 1000;
    if (countdownSeconds > 0) {
      countdownSeconds--;
    }
    if (countdownSeconds == 0) {
      countingDown = false;
      transitionReady = true;
      // Mostrar mensaje final
      mensajeTemporal = "Sistema Armado";
      mensajeUntilMillis = millis() + 2000;
      // Llamada al manejador de cambio de estado (se espera implementación en sketch)
      cambiarEstado(nullptr);
    }
  }
}

bool EstadoDesarmado::validarPIN() {
  return pinIngresado == PIN_CORRECTO;
}

unsigned int EstadoDesarmado::obtenerTiempoCronometro() const { return pot1Value; }
unsigned int EstadoDesarmado::obtenerTiempoAlarma() const { return pot2Value; }
bool EstadoDesarmado::estaConfigurado() const { return (lastRawPot1 >= 0 && lastRawPot2 >= 0); }
bool EstadoDesarmado::debeTransicionarArmado() const { return transitionReady; }

void EstadoDesarmado::reiniciar() {
  pinIngresado = "";
  countingDown = false;
  countdownSeconds = 0;
  transitionReady = false;
}

// Configura los pines usados por este estado
void EstadoDesarmado::configurarPines() {
  // Pines de displays
  pinMode(DISPLAY_1_EN, OUTPUT);
  pinMode(DISPLAY_2_EN, OUTPUT);
  pinMode(DATA_BIT_0, OUTPUT);
  pinMode(DATA_BIT_1, OUTPUT);
  pinMode(DATA_BIT_2, OUTPUT);
  pinMode(DATA_BIT_3, OUTPUT);

  // LEDs o indicadores si se requieren (opcional)
  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, HIGH);

  // Potes configurados como entradas analógicas (no pinMode necesario)
}

// Implementación débil de cambiarEstado para evitar errores de enlace
// Si el sketch principal provee su propia función, ésta prevalecerá.
void cambiarEstado(void* nuevoEstado) __attribute__((weak));
void cambiarEstado(void* /*nuevoEstado*/) {
  // Placeholder: el sketch principal puede sobrescribir esta función.
}
