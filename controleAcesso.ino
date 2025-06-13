
#define COMMON_ANODE 

#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

constexpr uint8_t redLed = 27;   // Definir os pinos dos LEDs
constexpr uint8_t greenLed = 26;
constexpr uint8_t blueLed = 25;

constexpr uint8_t relay = 13;     // Definir pino do relé
constexpr uint8_t wipeB = 33;     // Pino do botão para o modo de limpeza (WipeMode)

boolean match = false;          // Inicializa a correspondência do cartão como falsa
boolean programMode = false;  // Inicializa o modo de programação como falso
boolean replaceMaster = false;

uint8_t successRead;    // Variável inteira para armazenar se houve leitura bem-sucedida do leitor

byte storedCard[4];   // Armazena um ID lido da EEPROM
byte readCard[4];     // Armazena o ID lido pelo módulo RFID
byte masterCard[4];   // Armazena o ID do cartão mestre lido da EEPROM

// Cria uma instância do MFRC522
constexpr uint8_t RST_PIN = 2;     // Configurável, veja o layout típico de pinos acima
constexpr uint8_t SS_PIN = 5;      // Configurável, veja o layout típico de pinos acima

MFRC522 mfrc522(SS_PIN, RST_PIN);

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  EEPROM.begin(1024);

  // Configuração dos pinos do Arduino
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);   // Habilita o resistor pull-up do pino
  pinMode(relay, OUTPUT);
  // Cuidado com o comportamento do circuito do relé ao reiniciar ou religar o Arduino
  digitalWrite(relay, LOW);    // Garante que a porta esteja trancada
  digitalWrite(redLed, LED_OFF);   // Garante que o LED esteja desligado
  digitalWrite(greenLed, LED_OFF); // Garante que o LED esteja desligado
  digitalWrite(blueLed, LED_OFF);  // Garante que o LED esteja desligado

  // Configuração dos protocolos
  Serial.begin(9600);     // Inicializa a comunicação serial com o PC
  SPI.begin();            // O hardware MFRC522 usa o protocolo SPI
  mfrc522.PCD_Init();     // Inicializa o hardware MFRC522

  // Se definir o ganho da antena como máximo, aumentará a distância de leitura
  // mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  Serial.println(F("Controle de Acesso v0.1"));   // Para fins de depuração
  ShowReaderDetails();  // Mostra detalhes do leitor de cartão MFRC522

  // Código de limpeza - Se o botão (wipeB) for pressionado enquanto o setup é executado (ao ligar),
  // ele limpa a EEPROM
  if (digitalRead(wipeB) == LOW) {  // quando o botão for pressionado, o pino deve ficar em nível baixo (botão ligado ao GND)
    digitalWrite(redLed, LED_ON); // LED vermelho permanece aceso para informar que a limpeza será feita
    Serial.println(F("Botão de formatação apertado"));
    Serial.println(F("Você tem 10 segundos para cancelar"));
    Serial.println(F("Isso vai apagar todos os seus registros e não tem como desfazer"));
    bool buttonState = monitorWipeButton(10000); // Dá tempo suficiente para o usuário cancelar a operação
    if (buttonState == true && digitalRead(wipeB) == LOW) {    // Se o botão ainda estiver pressionado, limpa a EEPROM
      Serial.println(F("Início da formatação da EEPROM"));
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    // Percorre até o fim do endereço da EEPROM
        if (EEPROM.read(x) == 0) {              // Se o endereço da EEPROM já for 0
          // não faz nada, já está limpo, vai para o próximo endereço para economizar tempo e reduzir escritas na EEPROM
        }
        else {
          EEPROM.write(x, 0);       // se não, escreve 0 para limpar (leva 3.3ms)
        }
      }
      Serial.println(F("EEPROM formatada com sucesso"));
      digitalWrite(redLed, LED_OFF);  // Visualiza a limpeza bem-sucedida
      delay(200);
      digitalWrite(redLed, LED_ON);
      delay(200);
      digitalWrite(redLed, LED_OFF);
      delay(200);
      digitalWrite(redLed, LED_ON);
      delay(200);
      digitalWrite(redLed, LED_OFF);
    }
    else {
      Serial.println(F("Formatação cancelada")); // Mostra que o botão não foi pressionado por tempo suficiente
      digitalWrite(redLed, LED_OFF);
    }
  }

  // Verifica se o cartão mestre está definido, se não estiver, permite que o usuário escolha um cartão mestre
  // Isso também é útil para redefinir o cartão mestre
}


  if (EEPROM.read(1) != 143) {
    Serial.println(F("Cartão Mestre não definido"));
    Serial.println(F("Aproxime um chip para definir o Cartão Mestre"));
    do {
      successRead = getID();            // define successRead como 1 quando há leitura do leitor, caso contrário 0
      digitalWrite(blueLed, LED_ON);    // Visualiza que o Cartão Mestre precisa ser definido
      delay(200);
      digitalWrite(blueLed, LED_OFF);
      delay(200);
    }
    while (!successRead);              // O programa não prossegue até uma leitura bem-sucedida
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 vezes
      EEPROM.write( 2 + j, readCard[j] );     // Escreve o UID do chip lido na EEPROM a partir do endereço 2
    }
    EEPROM.write(1, 143);              // Escreve na EEPROM indicando que o Cartão Mestre foi definido
    Serial.println(F("Cartão Mestre definido"));
  }

  Serial.println(F("-------------------"));
  Serial.println(F("UID do Cartão Mestre"));
  for ( uint8_t i = 0; i < 4; i++ ) {          // Lê o UID do Cartão Mestre da EEPROM
    masterCard[i] = EEPROM.read(2 + i);        // Armazena em masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Tudo está pronto"));
  Serial.println(F("Aguardando pelos chips para serem lidos"));
  cycleLeds();    // Tudo pronto, fornece feedback ao usuário com os LEDs

  EEPROM.commit();
}


///////////////////////////////////////// Loop Principal ///////////////////////////////////
void loop () {
  do {
    successRead = getID();  // define successRead como 1 quando há leitura do leitor, caso contrário 0

    // Quando o dispositivo estiver em uso, se o botão de limpeza for pressionado por 10 segundos, inicia o reset do Cartão Mestre
    if (digitalRead(wipeB) == LOW) { // Verifica se o botão está pressionado
      // Visualiza que a operação normal foi interrompida, vermelho serve como alerta ao usuário
      digitalWrite(redLed, LED_ON);
      digitalWrite(greenLed, LED_OFF);
      digitalWrite(blueLed, LED_OFF);
      // Dá feedback ao usuário
      Serial.println(F("Botão de formatação apertado"));
      Serial.println(F("O Cartão Mestre será apagado em 10 segundos!"));
      bool buttonState = monitorWipeButton(10000); // Dá tempo ao usuário para cancelar a operação
      if (buttonState == true && digitalRead(wipeB) == LOW) { // Se o botão ainda estiver pressionado, limpa a EEPROM
        EEPROM.write(1, 0);               // Reseta o número mágico
        EEPROM.commit();
        Serial.println(F("Cartão Mestre desvinculado do dispositivo"));
        Serial.println(F("Pressione o botão de reset da placa para reprogramar o Cartão Mestre"));
        while (1);  // Para tudo, aguardando reset manual
      }
      Serial.println(F("Desvinculação do Cartão Mestre cancelada"));
    }

    if (programMode) {
      cycleLeds();  // Modo de programação: alterna entre as cores dos LEDs aguardando leitura de novo cartão
    }
    else {
      normalModeOn(); // Modo normal: LED azul ligado, os outros desligados
    }

  } while (!successRead); // O programa não prossegue até uma leitura bem-sucedida

  if (programMode) {
    if ( isMaster(readCard) ) { // Se o Cartão Mestre for lido novamente, sai do modo de programação
      Serial.println(F("Leitura do Cartão Mestre"));
      Serial.println(F("Saindo do modo de programação"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // Se o chip for conhecido, remove da EEPROM
        Serial.println(F("Conheço este chip, removendo..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      }
      else {  // Se o chip for desconhecido, adiciona à EEPROM
        Serial.println(F("Não conheço este chip, incluindo..."));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      }
    }
  }
  else {
    if ( isMaster(readCard)) {    // Se o chip lido for o Cartão Mestre, entra em modo de programação
      programMode = true;
      Serial.println(F("Olá Mestre - Modo de programação iniciado"));
      uint8_t count = EEPROM.read(0);   // Lê o primeiro byte da EEPROM que armazena o número de registros
      Serial.print(F("Existem "));
      Serial.print(count);
      Serial.print(F(" registro(s) na EEPROM"));
      Serial.println("");
      Serial.println(F("Leia um chip para adicionar ou remover da EEPROM"));
      Serial.println(F("Leia o Cartão Mestre novamente para sair do modo de programação"));
      Serial.println(F("-----------------------------"));
    }
    else {
      if ( findID(readCard) ) { // Se não for o mestre, verifica se o chip está registrado
        Serial.println(F("Bem-vindo, você pode passar"));
        granted(300);  // Destrava a porta por 300 ms
      }
      else {  // Se o chip não for válido
        Serial.println(F("Você não pode passar"));
        denied();  // Acesso negado
      }
    }
  }
}



///////////////////////////////////////// Acesso Concedido ///////////////////////////////////
void granted ( uint16_t setDelay) {
  digitalWrite(blueLed, LED_OFF);   // Desliga o LED azul
  digitalWrite(redLed, LED_OFF);    // Desliga o LED vermelho
  digitalWrite(greenLed, LED_ON);   // Liga o LED verde
  digitalWrite(relay, HIGH);        // Destrava a porta!
  delay(setDelay);                  // Mantém a porta destravada pelo tempo definido
  digitalWrite(relay, LOW);         // Trava a porta novamente
  delay(1000);                      // Mantém o LED verde ligado por 1 segundo
}

///////////////////////////////////////// Acesso Negado ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Garante que o LED verde esteja desligado
  digitalWrite(blueLed, LED_OFF);   // Garante que o LED azul esteja desligado
  digitalWrite(redLed, LED_ON);     // Liga o LED vermelho
  delay(1000);
}

///////////////////////////////////////// Obtem o UID do chip (PICC) ///////////////////////////////////
uint8_t getID() {
  // Preparando para ler os chips (PICC)
  if ( ! mfrc522.PICC_IsNewCardPresent()) { // Se não houver novo cartão no leitor, retorna
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   // Se não conseguir ler o serial do cartão, retorna
    return 0;
  }
  // Alguns cartões Mifare têm UID de 4 ou 7 bytes. Aqui assumimos 4 bytes
  // Até suportarmos cartões de 7 bytes
  Serial.println(F("UID do chip lido:"));
  for ( uint8_t i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Para a leitura
  return 1;
}

void ShowReaderDetails() {
  // Obtém a versão do software do MFRC522
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("Versão do software MFRC522: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (desconhecida), provavelmente um clone chinês?"));
  Serial.println("");
  // Se retornar 0x00 ou 0xFF, provavelmente a comunicação falhou
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("ALERTA: Falha na comunicação, o módulo MFRC522 está conectado corretamente?"));
    Serial.println(F("SISTEMA ABORTADO: Verifique as conexões."));
    // Visualiza que o sistema está parado
    digitalWrite(greenLed, LED_OFF);  // Garante que o LED verde esteja desligado
    digitalWrite(blueLed, LED_OFF);   // Garante que o LED azul esteja desligado
    digitalWrite(redLed, LED_ON);     // Liga o LED vermelho
    while (true); // trava o sistema até reiniciar
  }
}

///////////////////////////////////////// Ciclando LEDs (Modo de Programação) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LED_OFF);    // Garante que o LED vermelho esteja desligado
  digitalWrite(greenLed, LED_ON);   // Liga o LED verde
  digitalWrite(blueLed, LED_OFF);   // Garante que o LED azul esteja desligado
  delay(200);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(blueLed, LED_ON);    // Liga o LED azul
  delay(200);
  digitalWrite(redLed, LED_ON);     // Liga o LED vermelho
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(blueLed, LED_OFF);
  delay(200);
}

///////////////////////////////////////// LEDs no Modo Normal ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);    // LED azul ligado indica pronto para leitura
  digitalWrite(redLed, LED_OFF);    // Garante que o LED vermelho esteja desligado
  digitalWrite(greenLed, LED_OFF);  // Garante que o LED verde esteja desligado
  digitalWrite(relay, LOW);         // Garante que a porta esteja trancada
}

///////////////////////////////////////// Lê um ID da EEPROM ///////////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Calcula a posição inicial
  for ( uint8_t i = 0; i < 4; i++ ) {   // Loop 4 vezes para obter os 4 bytes
    storedCard[i] = EEPROM.read(start + i);   // Atribui os valores lidos à array
  }
}

///////////////////////////////////////// Adiciona um ID na EEPROM ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Antes de escrever, verifica se o ID já existe na EEPROM
    uint8_t num = EEPROM.read(0);      // Lê o número de posições usadas (posição 0 guarda o número de IDs)
    uint8_t start = ( num * 4 ) + 6;   // Calcula onde começa o próximo slot
    num++;                             // Incrementa o contador
    EEPROM.write( 0, num );            // Atualiza o contador na EEPROM
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 vezes
      EEPROM.write( start + j, a[j] );   // Escreve os valores do array na posição correta da EEPROM
    }
    EEPROM.commit();
    successWrite();
    Serial.println(F("ID adicionado na EEPROM com sucesso"));
  }
  else {
    failedWrite();
    Serial.println(F("Erro! Tem alguma coisa errada com o ID do chip ou problema na EEPROM"));
  }
}



///////////////////////////////////////// Remover ID da EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Antes de deletar da EEPROM, verifica se temos esse cartão!
    failedWrite();          // Se não tiver
    Serial.println(F("Erro! Tem alguma coisa errada com o ID do chip ou problema na EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Lê o número de espaços usados; a posição 0 armazena a quantidade de cartões
    uint8_t slot;       // Determina o número do slot do cartão
    uint8_t start;      
    uint8_t looping;    // Número de vezes que o loop vai repetir
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Lê o primeiro byte da EEPROM que armazena o número de cartões
    slot = findIDSLOT( a );   // Determina o número do slot do cartão a ser removido
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrementa o contador em 1
    EEPROM.write( 0, num );   // Escreve o novo valor do contador
    for ( j = 0; j < looping; j++ ) {         // Loop para mover os dados
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Move os valores 4 posições para trás na EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {       // Loop de limpeza
      EEPROM.write( start + j + k, 0);        // Limpa os dados restantes
    }
    EEPROM.commit();
    successDelete();
    Serial.println(F("ID removido da EEPROM com sucesso"));
  }
}

///////////////////////////////////////// Verificar Bytes   ///////////////////////////////////
// Verifica se dois arrays de bytes são iguais
boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != 0 )           // Garante que há algo no array
    match = true;            // Assume que são iguais
  for ( uint8_t k = 0; k < 4; k++ ) { // Loop de comparação
    if ( a[k] != b[k] )      // Se algum byte for diferente, não são iguais
      match = false;
  }
  if ( match ) {             // Verifica se ainda são iguais
    return true;
  }
  else  {
    return false;
  }
}

///////////////////////////////////////// Encontrar Slot   ///////////////////////////////////
// Retorna o número do slot de um ID passado
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Lê o primeiro byte da EEPROM
  for ( uint8_t i = 1; i <= count; i++ ) {    // Percorre todos os IDs armazenados
    readID(i);                // Lê um ID da EEPROM para storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Verifica se é igual ao buscado
      return i;         // Retorna o número do slot
      break;            // Para a busca
    }
  }
}

///////////////////////////////////////// Encontrar ID na EEPROM   ///////////////////////////////////
// Verifica se o ID existe na EEPROM
boolean findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Lê o primeiro byte da EEPROM
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop para todos os IDs
    readID(i);          // Lê um ID da EEPROM
    if ( checkTwo( find, storedCard ) ) {   // Verifica se o ID é igual
      return true;
      break;  
    }
  }
  return false;   // Se não encontrou, retorna false
}

///////////////////////////////////////// Sucesso ao Gravar na EEPROM   ///////////////////////////////////
// Pisca o LED verde 3 vezes para indicar sucesso ao gravar
void successWrite() {
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(greenLed, LED_ON);
  delay(200);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(greenLed, LED_ON);
  delay(200);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(greenLed, LED_ON);
  delay(200);
}

///////////////////////////////////////// Falha ao Gravar na EEPROM   ///////////////////////////////////
// Pisca o LED vermelho 3 vezes para indicar erro ao gravar
void failedWrite() {
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(redLed, LED_ON);
  delay(200);
  digitalWrite(redLed, LED_OFF);
  delay(200);
  digitalWrite(redLed, LED_ON);
  delay(200);
  digitalWrite(redLed, LED_OFF);
  delay(200);
  digitalWrite(redLed, LED_ON);
  delay(200);
}

///////////////////////////////////////// Sucesso ao Remover UID da EEPROM  ///////////////////////////////////
// Pisca o LED azul 3 vezes para indicar sucesso ao apagar
void successDelete() {
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(blueLed, LED_ON);
  delay(200);
  digitalWrite(blueLed, LED_OFF);
  delay(200);
  digitalWrite(blueLed, LED_ON);
  delay(200);
  digitalWrite(blueLed, LED_OFF);
  delay(200);
  digitalWrite(blueLed, LED_ON);
  delay(200);
}

////////////////////// Verifica se readCard é o cartão mestre   ///////////////////////////////////
// Compara o cartão lido com o cartão mestre
boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) )
    return true;
  else
    return false;
}

////////////////////// Verifica botão de limpeza (wipe)   ///////////////////////////////////
// Verifica se o botão de limpar foi pressionado por tempo suficiente
bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // verifica a cada meio segundo
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}



