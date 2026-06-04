#include <WiFi.h>
#include <HTTPClient.h>

// Credenciales de tu red Wi-Fi (Comparte internet desde tu celular para la prueba)
const char* ssid = "Redmi Note 13 Pro";
const char* password = "Daniel12";

// Datos de CallMeBot
const String phoneNumber = "+573005474611";
const String apiKey = "TU_API_KEY";

void setup() {
  // Serial para monitorear en el PC
  Serial.begin(115200); 
  
  // Serial2 para escuchar al Arduino MEGA (RX en pin 16, TX en pin 17)
  Serial2.begin(9600, SERIAL_8N1, 16, 17); 

  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado.");
}

void loop() {
  // Verificar si el MEGA envió algo
  if (Serial2.available()) {
    String mensajeRecibido = Serial2.readStringUntil('\n');
    mensajeRecibido.trim(); // Limpiar espacios ocultos o retornos de carro

    Serial.println("Recibido del MEGA: " + mensajeRecibido);

    // Enrutar el mensaje
    if (mensajeRecibido == "PANICO_REAL") {
      enviarWhatsApp("ALERTA: Botón de pánico activado.");
    } 
    else if (mensajeRecibido == "PANICO_PRUEBA") {
      enviarWhatsApp("PRUEBA: Botón de pánico activado.");
    } 
    else if (mensajeRecibido == "INTRUSION_REAL") {
      enviarWhatsApp("ALERTA: Sistema de alarma vulnerado.");
    } 
    else if (mensajeRecibido == "INTRUSION_PRUEBA") {
      enviarWhatsApp("PRUEBA: Sensor vulnerado.");
    }
  }
}

// Función para ejecutar el Webhook
void enviarWhatsApp(String mensaje) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Codificar URL para soportar espacios y caracteres especiales
    mensaje.replace(" ", "+");
    
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&text=" + mensaje + "&apikey=" + apiKey;
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
      Serial.print("Mensaje enviado con éxito. Código HTTP: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error enviando mensaje. Código HTTP: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Error: Wi-Fi desconectado.");
  }
}