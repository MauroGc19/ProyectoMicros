#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SevSeg.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"
#include "EstadoDesarmado.h"
#include "armado.h"


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
// EstadoArmado estadoArmado;
// EstadoActivado estadoActivado;
// Cronometro cronometro;

// ===================== Variables Globales =====================

// Estados del sistema
enum EstadoSistema {
  ESTADO_DESARMADO,
  ESTADO_ARMADO,
  ESTADO_ALARMA
};

EstadoSistema estadoActual = ESTADO_DESARMADO;
EstadoSistema estadoAnterior = ESTADO_DESARMADO;

// Parámetros del sistema (se configuran en DESARMADO)
unsigned int tiempoCronometro = 30;
unsigned int tiempoAlarma = 30;

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
  Serial.print("RAW POT1:"); Serial.print(raw_pot1); Serial.print(" M1:"); Serial.print(mapped1);
  Serial.print(" | RAW POT2:"); Serial.print(raw_pot2); Serial.print(" M2:"); Serial.println(mapped2);
  
  // Actualizar estado actual
  if (estadoActual == ESTADO_DESARMADO) {
    estadoDesarmado.update();
    
    // Verificar si debe transicionar a armado
    if (estadoDesarmado.debeTransicionarArmado()) {
      Serial.println("=== TRANSICIÓN A ESTADO ARMADO ===");
      estadoDesarmado.exit();
      estadoActual = ESTADO_ARMADO;
      // estadoArmado.enter();
      
      // Mostrar parámetros configurados
      Serial.print("Parámetros configurados - Cronometro: ");
      Serial.print(tiempoCronometro);
      Serial.print("s, Alarma: ");
      Serial.print(tiempoAlarma);
      Serial.println("s");
    }
  }
  // Aquí irán las lógicas de ESTADO_ARMADO y ESTADO_ALARMA cuando se implementen
  
  delay(50); // Pequeño delay para evitar saturar el procesador
}

// ===================== Funciones de Utilidad =====================

void mostrarValoresPotenciometros() {
  int pot1 = analogRead(POT_1);
  int pot2 = analogRead(POT_2);
  
  Serial.print("POT1: ");
  Serial.print(pot1);
  Serial.print(" -> ");
  Serial.print(tiempoCronometro);
  Serial.print("s | POT2: ");
  Serial.print(pot2);
  Serial.print(" -> ");
  Serial.print(tiempoAlarma);
  Serial.println("s");
}