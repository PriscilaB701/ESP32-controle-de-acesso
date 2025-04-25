# ESP32-controle-de-acesso
1. Objetivo
   
Desenvolver um sistema de controle de acesso baseado no ESP32, utilizando tecnologia RFID para identificação de usuários autorizados. O projeto integra uma interface web para monitoramento remoto dos acessos, conectando o sistema a uma rede Wi-Fi, caracterizando um sistema ciberfísico moderno e funcional.

3. Justificativa
   
A crescente demanda por sistemas de segurança física exige soluções práticas, eficientes e conectadas. Este projeto visa aliar a identificação por RFID com a conectividade do ESP32 para permitir o controle e o monitoramento de acessos em tempo real via internet local. A aplicação se insere no contexto de sistemas ciberfísicos por integrar sensores físicos, controle lógico embarcado e comunicação em rede.

5. Tecnologias Utilizadas
     
  ESP32 NodeMCU (microcontrolador Wi-Fi/Bluetooth)
  
  Módulo RFID RC522 (leitor de cartões/chaveiros RFID)
  
  Cartões e chaveiros RFID (frequência 13.56 MHz)
  
  Protoboard e jumpers
  
  MFRC522.h (para comunicação RFID via SPI)
  
  ESPAsyncWebServer.h (servidor web assíncrono)
  
  AsyncTCP.h (TCP assíncrono para ESP32)
  
  Protocolos de Comunicação
  SPI (entre ESP32 e RFID)
  
  HTTP (servidor web local)

4. Arquitetura Geral do Sistema
   
Descrição

  O módulo RFID RC522 detecta cartões e transmite os dados via SPI ao ESP32.
  
  O ESP32 processa o UID lido e verifica se o cartão está autorizado.
  
  O resultado da verificação (acesso liberado ou negado) é exibido em uma interface web hospedada no próprio ESP32.
  
  A interface web é acessada via Wi-Fi, permitindo o monitoramento remoto dos acessos.

Fluxograma simples

  Cartão RFID → RC522 → ESP32 → Verificação → Interface Web

5. Cronograma de Execução

                   Atividade | Status
        1 | Definição do problema e justificativa do projeto | Concluído
        2 | Montagem do circuito (ESP32 + RFID) | Concluído
        3 | Implementação da leitura RFID | Concluído
        4 | Implementação do servidor web no ESP32 | Em andamento
        5 | Integração e testes dos módulos (RFID + Web) | Pendente
        6 | Documentação do projeto e preparação da apresentação final | Pendente


7. Testes Isolados
   
  Módulo RFID RC522: Testado isoladamente com ESP32 para leitura dos UIDs dos cartões.
  
  Servidor Web: Testado isoladamente com ESP32, exibindo página de status sem RFID.
  
  Comunicação SPI: Validada entre ESP32 e RC522.
  
  Wi-Fi: Validação da conectividade e servidor HTTP.
