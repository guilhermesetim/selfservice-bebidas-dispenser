# Autosserviço de bebidas - Dispenser

Protótipo de um sistema de autosserviço de bebida baseado em crédito para o usuário, com tag de identificação. Integrado com um sistema de gestão para o proprietário.

O fluxo de funcionamento consiste em o cliente cadastra-se no caixa a sua tag ID, e a partir desse cadastro realizar a compra dos créditos. O cliente ficaria em posse dessa tag, e poderá se servir de forma autônoma em um dos dispensers, desde que possua os créditos. Dessa forma, o equipamento proporcionará agilidade para o cliente e automação para o negócio. 

O Sistema de dispenser por meio de um módulo RFID, realizará uma consulta ao banco de dados, que por sua vez, retorna o nome do usuário, e seu respectivo crédito no display, e no caso do usuário não possuir créditos, impedir o fornecimento da bebida. O sensor de fluxo identifica a quantidade de líquido consumida em ml, e em quanto atingir o limite estabelecido interrompe o fornecimento e atualiza o crédito do usuário.

O sofware de gestão do proprietário, tem o objetivo de entregar um balancete do estabelecimento, para facilitar a gestão e controle do estabelecimento, por meio de uma aplicação web, assim permitindo que o gestor acessar remotamente. O sistema apresenta quatro áreas de informações, sendo elas: Balanço geral do estabelecimento; Vandas realizadas no caixa; Consumo dos usuários no dispenser; e o registro das despesas do estabelecimento. 

Esse repositório destina-se ao *dispenser* do protótipo, o projeto deve ser executado junto com os seguintes repositórios: 
- Banco de dados:
- Caixa:
- Software de gestão:

## Arquitetura do Dispenser
O dispenser é o sistema mais complexo do projeto, pois é o que mais requisita de hardwares. A entrada do sistema consiste na leitura da tag do usuário por meio do módulo RFID, o módulo RFID comunica com o microcontrolador para enviar o código ID da tag. Por sua vez, o microcontrolador realiza o tratamento das informações de byte para hexadecimal, e envia ao servidor o código via porta serial. 

O servidor realiza o tratamento da informação recebida e realiza a consulta ao banco de dados, e retorna as informações da consulta ao microcontrolador. O microcontrolador mostra a saída das informações por meio do display LCD 16x2, juntamente ativa a válvula solenoide e começa a receber os bits do sensor de fluxo conforme passa o chopp e assim chegando até o usuário. No momento em que atingir o limite a válvula impedirá o fornecimento do chopp e descontará um crédito do usuário no banco de dados.

### Hardware utilizado
- Arduino Uno;
- Cabo serial arduino Uno;
- RFID MFRC-522;
- Display LCD 16x2 I2C;
- Protoboard;
- Sensor de fluxo de água;
- LED (simulando uma válvula solenóide);

### Software
#### Microcontrolador
- Linguagem de programação: C/C++;
- Bibliotecas:
  - LiquidCrystal_I2C
  - MFRC522.
 
#### Servidor
- Linguagem de programação: Python
- Bibliotecas:
  - PySerial
  A biblioteca pode ser encontra em: https://pyserial.readthedocs.io/en/latest/index.html



## Como executar o projeto
O microcontrolador deve ser carregado por meio do Arduino IDE, com o arquivo dispenser_arduino.ino;

O servidor ser executado com o python, com o arquivo dispenser_servidor.py

### Como adequar ao seu projeto de ChatBot
1. No arquivo do microcontrolador, contém uma variavel para calibragem, que sofre alteração a depender do calibre no condutor até o sensor de fluxo, e o valor limite que o dispenser utiliza para suspender o fornecimento. E na sequência, contém as constantes dos pinos do microconrolador.

2. No servidor é necessário execuar o banco de dados MySQL, e informar o endereço e senha do banco de dados utilizado. Além de informar a porta USB que o microcontrolador está executando, para Windows utiliza-se 'COM' e para sistemas UNIX  '/dev/ttyUSB', ambos seguidos do número da porta.

# Autor
Guilherme Setim
