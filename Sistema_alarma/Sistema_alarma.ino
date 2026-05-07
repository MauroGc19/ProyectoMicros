#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SevSeg.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"
#include "desarmado.h"
#include "armado.h"
#include "activado.h"
#include "cronometro.h"

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
Armado estadoArmado;

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
  // 1. Lectura de hardware crítica (No bloqueante y sin delays)
  char teclaPresionada = teclado.getKey();
  bool botonPanicoPresionado = (digitalRead(BUTTON_1) == LOW); // Asumiendo lógica pull-up

  // 2. Interrupción manual por Botón de Pánico
  if (botonPanicoPresionado) {
      Serial.println("¡PÁNICO ACTIVADO!");
      estadoActual = ESTADO_ARMADO; // Forzar el cambio de estado si estaba desarmado
      estadoArmado.triggerPanico(); // Llama a tu método
  }

  // 3. Máquina de Estados (Patrón State)
  switch (estadoActual) {
      
    case ESTADO_DESARMADO:
      if (teclaPresionada) {
        estadoDesarmado.procesarTecla(teclaPresionada);
      }
      estadoDesarmado.update();
      
      if (estadoDesarmado.debeTransicionarArmado()) {
        Serial.println("=== TRANSICIÓN A ESTADO ARMADO ===");
        estadoDesarmado.exit();
        estadoActual = ESTADO_ARMADO;
        estadoArmado.activarArmado(); // Llama a la inicialización de tu clase
      }
      break;

    case ESTADO_ARMADO:
      // Aquí el motor de tu clase respira y evalúa sensores/tiempos
      estadoArmado.update();

      // Validación para PODER DESARMAR
      if (teclaPresionada) {
        Serial.print("Tecla en estado armado: ");
        Serial.println(teclaPresionada);
        
        // AQUÍ tu equipo DEBE implementar la lógica que recibe la clave.
        // Asumiendo que tienen una función 'validarPinDesarmado(char tecla)'
        // bool pinCorrecto = validarPinDesarmado(teclaPresionada);
        
        // Si el pin es correcto:
        /*
        if (pinCorrecto) {
            estadoArmado.desactivarArmado(); // Tu método limpia los LEDs y el Buzzer
            estadoActual = ESTADO_DESARMADO;
            // estadoDesarmado.enter();
        }
        */
      }
      break;

    case ESTADO_ALARMA:
      // Si deciden usar este estado aparte de ARMADO, tu clase debe controlarlo,
      // pero en nuestra arquitectura, ESTADO_ARMADO ya maneja la alarma disparada.
      // Evalúen si realmente necesitan 3 estados o si con 2 (Desarmado / Armado-Vigilando) basta.
      break;
  }
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