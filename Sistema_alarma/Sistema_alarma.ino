#include "EstadoArmado.h"
#include "EstadoDesarmado.h"
#include "pins.h"
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SevSeg.h>

// ===================== Instancias Globales =====================
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4,
                  LCD_D5, LCD_D6, LCD_D7);

const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {{'1', '2', '3', 'A'},
                                {'4', '5', '6', 'B'},
                                {'7', '8', '9', 'C'},
                                {'#', '0', '*', 'D'}};

byte pinFilas[FILAS] = {KEYPAD_ROW_1, KEYPAD_ROW_2, KEYPAD_ROW_3, KEYPAD_ROW_4};
byte pinColumnas[COLUMNAS] = {KEYPAD_COL_1, KEYPAD_COL_2, KEYPAD_COL_3,
                              KEYPAD_COL_4};
Keypad teclado =
    Keypad(makeKeymap(teclas), pinFilas, pinColumnas, FILAS, COLUMNAS);

EstadoDesarmado estadoDesarmado;
EstadoArmado estadoArmado;

// ===================== Variables Globales =====================
enum EstadoSistema { ESTADO_DESARMADO, ESTADO_ARMADO ESTADO_ARMADO };

EstadoSistema estadoActual = ESTADO_DESARMADO;
EstadoSistema estadoAnterior = ESTADO_DESARMADO;

volatile bool boton1InterruptEvent = false;
volatile bool boton2InterruptEvent = false;
volatile bool led8Latched = false;

void boton1ISR() {
  // marca evento
  boton1InterruptEvent = true;
}

void boton2ISR() {
  // Solicita transición a estado DESARMADO
  boton2InterruptEvent = true;
}

// Parámetros del sistema (se configuran en DESARMADO)
unsigned int tiempoCronometro = 30;
unsigned int tiempoAlarma = 30;

// Prototipo para función de cambio de estado usada por los estados
extern void cambiarEstado(void *nuevoEstado);

// Implementación central de cambio de estado
void cambiarEstado(void *nuevoEstado) {
  // nuevoEstado == nullptr -> transición a ARMADO (solicitada por
  // EstadoDesarmado)
  if (nuevoEstado == nullptr) {
    estadoDesarmado.exit();
    estadoActual = ESTADO_ARMADO;
    estadoArmado.enter();
    return;
  }
  // nuevoEstado == (void*)1 -> transición a DESARMADO (señal desde
  // EstadoArmado)
  if (nuevoEstado == (void *)1) {
    // Asegurar que el relay y latches se apaguen al desarmar
    digitalWrite(RELAY_PIN, LOW);
    led8Latched = false;
    estadoDesarmado.reiniciar();
    estadoActual = ESTADO_DESARMADO;
    estadoDesarmado.enter();
    return;
  }
  // Otros valores ignorados por ahora
}

// ===================== Setup =====================
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("=== SISTEMA DE ALARMA INICIANDO ===");

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Sistema Iniciando");

  pinMode(POT_1, INPUT);
  pinMode(POT_2, INPUT);
  pinMode(SWITCH_1, INPUT);
  pinMode(SWITCH_2, INPUT);
  pinMode(SWITCH_3, INPUT);
  pinMode(SWITCH_4, INPUT);
  pinMode(SWITCH_5, INPUT);
  pinMode(SWITCH_6, INPUT);
  pinMode(SWITCH_7, INPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), boton1ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), boton2ISR, FALLING);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(DISPLAY_1_EN, OUTPUT);
  pinMode(DISPLAY_2_EN, OUTPUT);
  pinMode(DATA_BIT_0, OUTPUT);
  pinMode(DATA_BIT_1, OUTPUT);
  pinMode(DATA_BIT_2, OUTPUT);
  pinMode(DATA_BIT_3, OUTPUT);

  for (int i = 0; i < 8; i++) {
    int pinLED = 2 + i;
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, LOW);
  }

  estadoActual = ESTADO_DESARMADO;
  // estadoDesarmado.enter(); // Descomentar cuando la clase EstadoDesarmado
  // esté lista

  Serial.println("Sistema inicializado");
}

// ===================== Loop Principal =====================
void loop() {
  // Depuración: mostrar lecturas crudas y mapeadas de potenciómetros
  int raw_pot1 = analogRead(POT_1);
  int raw_pot2 = analogRead(POT_2);
  int mapped1 = map(raw_pot1, 0, 1023, 0, 30);
  int mapped2 = map(raw_pot2, 0, 1023, 5, 30);

  // Actualizar estado actual
  if (estadoActual == ESTADO_DESARMADO) {
    estadoDesarmado.update();

    tiempoCronometro = estadoDesarmado.obtenerTiempoCronometro();
    tiempoAlarma = estadoDesarmado.obtenerTiempoAlarma();

    // Verificar si debe transicionar a armado
    if (estadoDesarmado.debeTransicionarArmado()) {
      Serial.println("=== TRANSICIÓN A ESTADO ARMADO ===");
      // Delegar la transición a la función central cambiarEstado
      cambiarEstado(nullptr);

      // Mostrar parámetros configurados
      Serial.print("Parámetros configurados - Cronometro: ");
      Serial.print(tiempoCronometro);
      Serial.print("s, Alarma: ");
      Serial.print(tiempoAlarma);
      Serial.println("s");
    }
  } else if (estadoActual == ESTADO_ARMADO) {
    estadoArmado.update();
  }

  // Si BUTTON_1 se pulsó, alternar el latch de LED_8.
  if (boton1InterruptEvent) {
    boton1InterruptEvent = false;

    // Forzar transición a estado armado si está desarmado
    if (estadoActual == ESTADO_DESARMADO) {
      cambiarEstado(nullptr); // Transiciona a ARMADO
    }

    // Detonar la alarma a través de la máquina de estados
    estadoArmado.activarSirena();

    if (digitalRead(SWITCH_7) == HIGH) {
      Serial1.println("PANICO_PRUEBA");
      Serial.println("Notificación: MODO PRUEBA ENVIADO");
    } else {
      Serial1.println("PANICO_REAL");
      Serial.println("Notificación: ALARMA REAL ENVIADA");
    }
  }

  // Si BUTTON_2 se pulsó, forzar transición a DESARMADO
  if (boton2InterruptEvent) {
    boton2InterruptEvent = false;
    cambiarEstado((void *)1);
  }

  // Lógica para LED_8 combinada:
  // - SWITCH_6 activa LED_8 mientras esté presionado
  // - BUTTON_1 alterna el estado de LED_8 hasta un nuevo pulso
  // - EstadoArmado puede activar LED_8 temporalmente cuando la alarma vence
  bool externalLed8 = digitalRead(SWITCH_6) == HIGH;
  bool armadoLed8 = estadoArmado.isLed8Active();
  bool sirenaActiva = estadoArmado.isSirenaActive();
  if (externalLed8 || led8Latched || armadoLed8 || sirenaActiva)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);

  // Aquí irán las lógicas de ESTADO_ARMADO cuando se implementen
  // Pequeño delay para evitar saturar el procesador
}

// ===================== Funciones de Utilidad =====================

void mostrarValoresPotenciometros() {
  int pot1 = analogRead(POT_1);
  int pot2 = analogRead(POT_2);
}