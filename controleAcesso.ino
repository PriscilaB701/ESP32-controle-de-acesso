#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>


const char* ssid = "priscila";          
const char* password = "P042020ps";      


#define SS_PIN 5   
#define RST_PIN 22 
MFRC522 mfrc522(SS_PIN, RST_PIN);


AsyncWebServer server(80);

String ultimaMensagem = "Aguardando leitura de cartão...";
String historicoAcessos = "";


String autorizados[] = {
  "04 A1 B2 C3", 
  "12 34 56 78"  
};

//verificar se o UID é autorizado
bool acessoLiberado(String uid) {
  for (String id : autorizados) {
    if (uid == id) {
      return true;
    }
  }
  return false;
}

//converte UID para String
String obterUID(MFRC522::Uid uid) {
  String conteudo = "";
  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] < 0x10) conteudo += "0";
    conteudo += String(uid.uidByte[i], HEX);
    if (i < uid.size-1) conteudo += " ";
  }
  conteudo.toUpperCase();
  return conteudo;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();


  WiFi.begin(ssid, password);
  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Controle de Acesso RFID</title></head><body>";
    html += "<h1>Controle de Acesso</h1>";
    html += "<h2>Status atual:</h2><p>" + ultimaMensagem + "</p>";
    html += "<h3>Histórico de acessos:</h3><pre>" + historicoAcessos + "</pre>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  // Aguarda novo cartão
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(200);
    return;
  }

  String uidLido = obterUID(mfrc522.uid);
  Serial.println("Cartão detectado: " + uidLido);

  if (acessoLiberado(uidLido)) {
    ultimaMensagem = "✅ Acesso liberado para UID: " + uidLido;
  } else {
    ultimaMensagem = "❌ Acesso negado para UID: " + uidLido;
  }

  historicoAcessos = ultimaMensagem + "\n" + historicoAcessos;

  mfrc522.PICC_HaltA();
  delay(1000); // evita leitura duplicada
}
