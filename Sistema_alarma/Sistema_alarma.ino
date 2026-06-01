#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SevSeg.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"
#include "EstadoDesarmado.h"
#include "EstadoArmado.h"


// ===================== Instancias Globales =====================

// LCD 16x2 - Inicialización con pines RS, EN, D0-D7
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Teclado Matricial 4x4
const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'#', '0', '*', 'D'}
};

byte pinFilas[FILAS] = {KEYPAD_ROW_1, KEYPAD_ROW_2, KEYPAD_ROW_3, KEYPAD_ROW_4};
byte pinColumnas[COLUMNAS] = {KEYPAD_COL_1, KEYPAD_COL_2, KEYPAD_COL_3, KEYPAD_COL_4};
Keypad teclado = Keypad(makeKeymap(teclas), pinFilas, pinColumnas, FILAS, COLUMNAS);

// Estados del sistema
EstadoDesarmado estadoDesarmado;
EstadoArmado estadoArmado;

// ===================== Variables Globales =====================

// Estados del sistema
enum EstadoSistema {
  ESTADO_DESARMADO,
  ESTADO_ARMADO
};

EstadoSistema estadoActual = ESTADO_DESARMADO;
EstadoSistema estadoAnterior = ESTADO_DESARMADO;

volatile bool boton1InterruptEvent = false;
volatile bool led8Latched = false;

void boton1ISR() {
  boton1InterruptEvent = true;
}

// Parámetros del sistema (se configuran en DESARMADO)
unsigned int tiempoCronometro = 30;
unsigned int tiempoAlarma = 30;

// Prototipo para función de cambio de estado usada por los estados
extern void cambiarEstado(void* nuevoEstado);

// Implementación central de cambio de estado
void cambiarEstado(void* nuevoEstado) {
  // nuevoEstado == nullptr -> transición a ARMADO (solicitada por EstadoDesarmado)
  if (nuevoEstado == nullptr) {
    estadoDesarmado.exit();
    estadoActual = ESTADO_ARMADO;
    estadoArmado.enter();
    return;
  }
  // nuevoEstado == (void*)1 -> transición a DESARMADO (señal desde EstadoArmado)
  if (nuevoEstado == (void*)1) {
    estadoDesarmado.reiniciar();
    estadoActual = ESTADO_DESARMADO;
    estadoDesarmado.enter();
    return;
  }
  // Otros valores ignorados por ahora
}

// ===================== Setup =====================

void setup() {
  // Inicializar comunicación serial para debugging
  Serial.begin(9600);
  Serial.println("=== SISTEMA DE ALARMA INICIANDO ===");
  
  // Inicializar LCD (16 columnas, 2 filas)
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Sistema Iniciando");
  
  // Configurar pines de entrada
  pinMode(POT_1, INPUT);
  pinMode(POT_2, INPUT);
  pinMode(SWITCH_1, INPUT);
  pinMode(SWITCH_2, INPUT);
  pinMode(SWITCH_3, INPUT);
  pinMode(SWITCH_4, INPUT);
  pinMode(SWITCH_5, INPUT);
  pinMode(SWITCH_6, INPUT);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), boton1ISR, FALLING);
  
  // Configurar pines de salida
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  pinMode(DISPLAY_1_EN, OUTPUT);
  pinMode(DISPLAY_2_EN, OUTPUT);
  pinMode(DATA_BIT_0, OUTPUT);
  pinMode(DATA_BIT_1, OUTPUT);
  pinMode(DATA_BIT_2, OUTPUT);
  pinMode(DATA_BIT_3, OUTPUT);
  
  // Configurar LEDs
  for (int i = 0; i < 8; i++) {
    int pinLED = 2 + i; // Pines 2-9
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, LOW);
  }
  
  // Inicializar el estado desarmado
  estadoActual = ESTADO_DESARMADO;
  estadoDesarmado.enter();
  
  Serial.println("Sistema inicializado correctamente");
  delay(1000);
}

// ===================== Loop Principal =====================

void loop() {
  // Leer tecla del teclado matricial
  char teclaPresionada = teclado.getKey();
  
  // Procesar la tecla en el estado actual
  if (teclaPresionada) {
    Serial.print("Tecla presionada: ");
    Serial.println(teclaPresionada);
    
    if (estadoActual == ESTADO_DESARMADO) {
      estadoDesarmado.procesarTecla(teclaPresionada);
      
      // Obtener parámetros configurados
      tiempoCronometro = estadoDesarmado.obtenerTiempoCronometro();
      tiempoAlarma = estadoDesarmado.obtenerTiempoAlarma();
    }
  }
  // Depuración: mostrar lecturas crudas y mapeadas de potenciómetros
  int raw_pot1 = analogRead(POT_1);
  int raw_pot2 = analogRead(POT_2);
  int mapped1 = map(raw_pot1, 0, 1023, 0, 30);
  int mapped2 = map(raw_pot2, 0, 1023, 5, 30);

  
  // Actualizar estado actual
  if (estadoActual == ESTADO_DESARMADO) {
    estadoDesarmado.update();
    
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
  }
  else if (estadoActual == ESTADO_ARMADO) {
    estadoArmado.update();
  }

  // Si BUTTON_1 se pulsó, alternar el latch de LED_8.
  if (boton1InterruptEvent) {
    led8Latched = !led8Latched;
    boton1InterruptEvent = false;
  }

  // Lógica para LED_8 combinada:
  // - SWITCH_6 activa LED_8 mientras esté presionado
  // - BUTTON_1 alterna el estado de LED_8 hasta un nuevo pulso
  // - EstadoArmado puede activar LED_8 temporalmente cuando la alarma vence
  bool externalLed8 = digitalRead(SWITCH_6) == HIGH;
  bool armadoLed8 = estadoArmado.isLed8Active();
  if (externalLed8 || led8Latched || armadoLed8) digitalWrite(RELAY_PIN, HIGH);
  else digitalWrite(RELAY_PIN, LOW);

  // Aquí irán las lógicas de ESTADO_ARMADO cuando se implementen
  // Pequeño delay para evitar saturar el procesador
}

// ===================== Funciones de Utilidad =====================

void mostrarValoresPotenciometros() {
  int pot1 = analogRead(POT_1);
  int pot2 = analogRead(POT_2);
}