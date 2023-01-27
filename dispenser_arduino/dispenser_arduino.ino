
// **********************************************************************
// **
// **	              Programa do dispenser de Chopp
// **
// **********************************************************************




//***********************************************************************
//	Bibliotecas:
//=======================================================================
#include <Wire.h>           
#include <SPI.h>                  
#include "LiquidCrystal_I2C.h"    // LCD I2C.
#include <MFRC522.h>              // Módulo RFID    







//***********************************************************************
//	Ajustes de calibragem e limite do serviço
//***********************************************************************
#define	fator_Vazao  2.25   // Calibrar esse parametro 
int limite_servico = 350;  // quantidade limite para encerrar o serviço
//***********************************************************************

//***********************************************************************
  // Constantes Globais para Hardware
//=======================================================================

// Sensor de Fluxo
#define	PINO_SENSOR_FLUXO   2   
// Led (Valvula Selenóide)
#define LED_DISPONIVEL 8 
// Módulo RFID
#define PINO_DADOS_MRFID 10 
#define PINO_RESET_MRFID 9 
// Display LCD
#define	LCD_colunas  16   // total de colunas do LCD.
#define	LCD_linhas  2   // total de linhas do LCD.
#define	LCD_ADDR  0x27   // endereco de acesso ao LCD no Barramento I2C.



//***********************************************************************
//	Define e instancia um objeto para acesso ao LCD I2C:
//=======================================================================

LiquidCrystal_I2C  Display_LCD ( LCD_ADDR, LCD_colunas, LCD_linhas );
MFRC522 mfrc522(PINO_DADOS_MRFID, PINO_RESET_MRFID); 

//***********************************************************************



//***********************************************************************
//	Variaveis relacionadas ao Sensor de Fluxo
//=======================================================================

float ml_instante = 0;   // Vazao instantanea obtida via Sensor de Fluxo.
float total_ml = 0;  // total consumido em Litros.
volatile unsigned long contador_PULSOS = 0;  // contagem atual dos pulsos do Sensor de Fluxo.
volatile bool habilita_Contagem = false;  // controle habilita/desabilita contagem dos pulsos.

//***********************************************************************




//***********************************************************************
//	Simulação de uma interrupção
//=======================================================================

void	ISR_Sensor_Fluxo ()
{
	if ( habilita_Contagem )  // se a contagem esta' habilitada:
	{
		contador_PULSOS++;  // incrementa a contagem de pulsos.
	}
}
//***********************************************************************




//***********************************************************************
//	Configurações do sistema
//=======================================================================

void	setup ()
{
// Inicializações  
	Serial.begin(9600);
	delay(500);
  SPI.begin();
  mfrc522.PCD_Init(); 
	Display_LCD.init(); 
	Display_LCD.backlight();

  // Configuração dos pinos
	pinMode( PINO_SENSOR_FLUXO, INPUT_PULLUP );
  pinMode(LED_DISPONIVEL, OUTPUT);
  // interrupção  
 	attachInterrupt( digitalPinToInterrupt( PINO_SENSOR_FLUXO ), ISR_Sensor_Fluxo, RISING ); 

  // Mensagem de abertura  
  mensagemDisplay("  Chopp", "  Self-Service");
	delay(2000); 
}

//***********************************************************************
//	Loop de Controle Principal do Sistema.
//=======================================================================


void	loop ()
{
  /*  Exibição inicial do dispenser   */
  mensagemDisplay("Por favor,","aproxime sua tag");
  delay(2000);
 

  /*  Leitura do ID    */
  while ( !leituraRFID() );
  String id= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
     id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "")); 
     id.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  // envio di ID para o servidor
  Serial.println(id);
 


  /*   Leitura das informações Serial     */
  // leitura canecas
  String qtd = "";
  while(!Serial.available() > 0);
  if(Serial.available() > 0){
    qtd = Serial.read();
  }
  
  // leitura do cod (não utilizado)
  while(!Serial.available() > 0);
  if(Serial.available() > 0);
  
  // leitura do nome
  String nome = "";
  int contagem = 0;
  while(contagem < 16) {
    static unsigned long ref_tempo_disponivel = millis();
    while( (!Serial.available() > 0) && ( millis() - ref_tempo_disponivel ) < 500 );
    if(Serial.available() > 0){
      nome += char(Serial.read());
    }
    ++contagem;
  }  
  
  
  /* 
    Verifica se o cliente possui saldo,
    senão reinicia o serviço            
  */
  if (!semSaldo(qtd.toInt()) ) {
    // Inicia o serviço
    servir(nome, qtd.toInt());
  }
  
  
}


//***********************************************************************
          // Funções
//***********************************************************************

bool leituraRFID() 
{
  // Aguarda a aproximacao do cartao
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  // Verifica se a tag foi aproximada no sensor
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  return true;
}


bool semSaldo(int copos)
{
  if (copos <= 0) {
    
    mensagemDisplay("Sem saldo", "Carregue seu ID");
    delay(3000);
    return true;
  }
  
  else {
    return false;
  }
    
}


void servir(String usuario, int copos) 
{
  /*    Inicialização do serviço    */
  mensagemDisplay(usuario, "Canecas: ");
  Display_LCD.print(copos);
  delay(1000);
  copos--;
  mensagemDisplay(usuario, "Canecas: ");
  Display_LCD.print(copos);
 
  delay(2000);

  digitalWrite(LED_DISPONIVEL, HIGH); // dispenser liberado

  bool servicoLiberado = true;
  while(servicoLiberado) {

    static bool startup = true;
    static bool calc_Fluxo = false;
    unsigned long sample_Contagem;
    static unsigned long ref_tempo_ms;

      /*  Configurações para funcionamento dos pulsos do Sensor de Fluxo  */
      if ( startup ) 
      {
        startup = false; 
        habilita_Contagem = false; 
        contador_PULSOS = 0;        
        ref_tempo_ms = millis();   
        habilita_Contagem = true;
    
        Display_LCD.clear(); 
        Display_LCD.setCursor( 0, 0 );
        Display_LCD.print(usuario);
      }
    

      /*    Temporização    */
      if ( ( millis() - ref_tempo_ms ) >= 1000 )
      {
        habilita_Contagem = false;        
        ref_tempo_ms = millis();       
        sample_Contagem = contador_PULSOS;        
        contador_PULSOS = 0;        
        habilita_Contagem = true;         
        calc_Fluxo = true; 
      }
    

      /*   Calculos matemáticos   */
      if ( calc_Fluxo ) 
      {
        calc_Fluxo = false;     
        ml_instante = (float) fator_Vazao * sample_Contagem;      
        total_ml += ml_instante; 
    
   
        /*    Display   */
        mensagemDisplay(usuario, F("    "));
        Display_LCD.print( total_ml );
        Display_LCD.setCursor( 10, 1 );
        Display_LCD.print( "ml" );

        /*    Reset dos elementos e finalização do serviço   */
        if(total_ml > limite_servico) {
          digitalWrite(LED_DISPONIVEL, LOW); // dispenser suspenso
          total_ml = 0;
          startup = true;
          
          mensagemDisplay("Obrigado", "Volte sempre");
          delay(5000);
          
          servicoLiberado = false;
        }
      }   
  }

}

void mensagemDisplay(String cima, String baixo)
{
    Display_LCD.clear();   
    Display_LCD.setCursor( 0, 0 );
    Display_LCD.print(cima);
    Display_LCD.setCursor( 0, 1 );
    Display_LCD.print(baixo);
}