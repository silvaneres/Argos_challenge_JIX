/*
 * Desafio Argos -> envio de caracteres via MQTT
 * Hardware utilizado: -> Esp-WROOM-32 (DEVKIT - V1)
 *                     -> Expansor de porta PCF8574 para comunicação LCD via I2C
 *                     -> Display LCD alfanumérico 16x2
 *                     -> Teclado matricial 4x4
 *                     
 * Projeto desenvolvido por Sérgio Silvaneres Pereira Silva                    
 * Mundo JIX
 * 
 * Desenvolvimento inicial do projeto
 * 
 */

// Inclusão de bibliotecas ==========================================================================

#include <LiquidCrystal_I2C.h>                         // Biblioteca de controle do LCD via I2C com o PCF8574
#include <Keypad.h>                                    //     ||           ||    do teclado matricial
#include <WiFi.h>                                      //     ||     de utilização WiFi para o Esp32
#include <PubSubClient.h>                              //     ||     de comunicação do MQTT
#include <ESP32Servo.h>

// Declaração de variáveis ==========================================================================

const int pin = 2;
const char* ssid = " ************* ";                  // nome da rede WiFi que será conectado
const char* password =  "**********";                  // Senha da rede Wifi
const char* mqttServer = "broker.mqtt-dashboard.com";  // Nome para o servidor Broker
const int mqttPort = 1883;                             // porta
const char* mqttUser = "abcdefg";                      // Usuário do cliente
const char* mqttPassword = "123456";                   // Senha de usuário do cliente
char mensagem[16];
const byte ROWS = 4;                                   // número de linhas do teclado matricial
const byte COLS = 4;                                   //  ||    de colunas     ||       ||
byte rowPins[ROWS] = {32, 33, 25, 26};                 // Pinos do Esp32 conectados às linhas     do teclado matricial
byte colPins[COLS] = {27, 14, 12, 13};                 //   ||      ||       ||     às colunas       ||         ||
volatile byte aux = 0;
String digitos = "";                                   // String para alocação dos dígitos 
char keys[ROWS][COLS] = {                              // Array que faz referência a posição de cada símbolo no teclado matricial
  {'D','C','B','A'},                                   //                             ||
  {'#','9','6','3'},                                   //                             ||
  {'0','8','5','2'},                                   //                             ||
  {'*','7','4','1'}                                    //                             ||
};
byte customChar[8] =                                   // Matriz do caractere especial de indicação 
{ 
  0b11000,                                             //                             ||
  0b11100,                                             //                             ||
  0b11110,                                             //                             ||
  0b11111,                                             //                             ||
  0b11110,                                             //                             ||
  0b11100,                                             //                             ||
  0b11000,                                             //                             ||
  0b00000                                              //                             ||
};

// Definições de hardware ===========================================================================

WiFiClient espClient;                            // Definição Nome Cliente
PubSubClient client(espClient);                  // Definição do espCliente como device para enviar e receber dados do broker
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );  // Definições para controle do teclado matricial
LiquidCrystal_I2C lcd(0x3F, 16, 2);              // Definição com o endereço de comu. I2C e definição de colunas e linhas do display


// Funções auxiliares ===============================================================================

byte estado_1()                                  // função estado_1 ( da máquina de estados )
{
  byte aux;                                      // declaro uma variável local
  conectabroker();                               // chamo a função para se conectar ao broker
  lcd.clear();                                   // Limpo o lcd
  lcd.setCursor(0, 0);                           // Seto o cursor de escrita no display 
  lcd.print("DIGITE A STRING");                  // Escrevo no display
  tone(pin, 1500, 300);                          // Emito um som de 1500 Hz
  tone(pin, 800, 300);                           // Aciono o buzzer em 800Hz
  delay(1000);                                   // travo o programa por 1 segundo
  return aux = 1;                                // retorno aux = 1 para execução da próxima função da máquina de estados
}// End byte estado_1()

byte estado_2(){                                 // função estado_2 ( da máquina de estados )
  byte aux;                                      // declaro uma variável local
  digitos = "";                                  // limpo o string digitos
  //tone(pin, 2000, 300);
  for( byte i = 0; i < 16; i ++ ){               // rotina de limpeza do array do payload
    mensagem[i] = ' ';                           // Limpo cada vetor
  }    
                                 
  lcd.clear();                                   // Limpo novamente o lcd para nova escrita
  lcd.setCursor(0, 1);                           // Seto o cursor de escrita no display
  lcd.print("C");                                // Escrevo no display
  lcd.setCursor(1, 1);                           // Seto o cursor de escrita no display
  lcd.write((byte)1);                            // imprimindo o carcter especial 1
  lcd.setCursor(2, 1);                           // Seto o cursor de escrita no display
  lcd.print("LIMPA  D");                         // Escrevo no display
  lcd.setCursor(10, 1);                          // Seto o cursor de escrita no display
  lcd.write((byte)1);                            // imprimindo o carcter especial 1
  lcd.setCursor(11, 1);                          // Seto o cursor de escrita no display
  lcd.print("ENVIA");                            // Escrevo no display
  lcd.setCursor(0, 0);                           // Seto o cursor de escrita no display
  lcd.blink();                                   // Ativo o pisca do cursor do display 
  return aux = 2;                                // retorno aux = 2 para execução da próxima função da máquina de estados
}// End byte estado_2( char key )

byte estado_3(){                                 // função estado_3 ( da máquina de estados )
  byte aux = 0;                                  // declaro uma variável local
  char key = keypad.getKey();                    // chamo a função de leitura do teclado e atribuo a uma nova variável
  if(key){                                       // se foi pressionado alguma tecla...
    switch(key){                                 // Se a tecla pressionada se relaciona com os casos abaixo:
      case 'C':                                  // Se foi digitado C
         lcd.noBlink();                          // Desligo o pisca do cursor
         tone(pin, 700, 300);                    // Aciono o buzzer em 700Hz
         aux = 1;                                // aux = 1 para retornar à função estado_2
         break;                                  // saio do switch
      case 'D':                                  // Se foi digitado D
         lcd.noBlink();                          // Desligo o pisca do cursor
         aux = 3;                                // aux = 3 para acionar a função estado_4 de envio de dados ao broker
         break;                                  // saio do switch
      default:                                   // Se foi digitado qualquer tecla diferente de C e de D...        
         digitos += key;                         // vou alocando os dígitos pressionados à string digitos 
         tone(pin, 1000, 200);                   // Aciono o buzzer em 1KHz
         lcd.print(key);                         // escrevo o digito pressionado no lcd
         aux = 2;                                // aux = 2 para retornar a essa rotina 
         break;                                  // saio do switch 
    }// End switch(key)
    return aux;                                  // saio da função estado_2 retornando o valor de aux
  }// End if(key)         
  else{                                          // Se foi pressionado nenhuma tecla...
    return aux = 2;                              // aux = 2 para retornar a essa rotina até que alguma tecla seja pressionada
  }
}// End byte estadp_3()

byte estado_4(){                                // função estado_4 ( da máquina de estados )
  byte aux;                                     // declaro uma variável local
  PbSubSystem();                                // Chamo a função que envia os dados ao broker
  return aux = 1;                               // Retorno aux=1 para recomeçar o loop e assim escrever o novo conteúdo 
}// End byte estadp_3()

void conectabroker()                                              // Função de conexao ao broker MQTT
{
  lcd.clear();                                                    // Limpo o Lcd   
  client.setServer(mqttServer, mqttPort);                         // Envio à rede o broker que será conectado e o port
  while (!client.connected())                                     // Enquanto o esp32 tenta se conectar ao broker...
  {
   lcd.setCursor(1, 0);                                           // Seto o cursor de escrita no display
   lcd.print(" CONECTANDO ");                                     // Escrevo outra mensagem
   lcd.setCursor(0, 1);                                           // Seto o cursor de escrita no display
   lcd.print("AO BROKER MQTT.");                                  // Escrevo outra mensagem
   tone(pin, 600, 300);                                           // Aciono o buzzer em 600Hz
   delay(700);
    if (client.connect("ESP32Client", mqttUser, mqttPassword ))   // Se houve conexão ao broker...
    { 
      lcd.clear();                                                // Limpo o Lcd  
      lcd.setCursor(2, 0);                                        // Seto o cursor de escrita no display
      lcd.print(" CONECTADO ");                                   // Escrevo outra mensagem
      lcd.setCursor(1, 1);                                        // Seto o cursor de escrita no display
      lcd.print(" AO BROKER!!!");                                 // Escrevo outra mensagem
      tone(pin, 1200, 300);                                       // Aciono o buzzer em 1200Hz
      tone(pin, 1200, 300);                                       //   ||       ||        ||
      delay(700);                                                 // delay para conexão( espera de resposta )
    }
    else                                                          // Se a tentativa de conexão falhou...
    {
      lcd.clear();                                                // Limpo o Lcd  
      lcd.setCursor(0, 0);                                        // Seto o cursor de escrita no display
      lcd.print("FALHA DE CONEXAO");                              // Escrevo outra mensagem
      lcd.setCursor(0, 1);                                        // Seto o cursor de escrita no display
      lcd.print(" AO BROKER...");                                 // Escrevo outra mensagem
      lcd.setCursor(15, 1);                                       // Seto o cursor de escrita no display
   //   Serial.print(client.state());
      delay(600);                                                 // Delay de 600ms
      lcd.clear();                                                // Limpo o LCD
    }
  }//End While(!client.conected)
}// End void conectabroker()

void reconectbroker(){                                            // Função para ficar reconectando ao broker
  lcd.clear();                                                    // Limpo o Lcd   
  client.setServer(mqttServer, mqttPort);                         // Envio à rede o broker que será conectado e o port
  while (!client.connected())                                     // Enquanto o esp32 tenta se conectar ao broker...
  {
    if (client.connect("ESP32Client", mqttUser, mqttPassword ))   // Se houve conexão ao broker...
    { 
     return;                                                      // saio da função
    }
    else                                                          // Se a tentativa de conexão falhou...
    {
      lcd.clear();                                                // Limpo o Lcd  
      lcd.setCursor(0, 0);                                        // Seto o cursor de escrita no display
      lcd.print("FALHA DE CONEXAO");                              // Escrevo outra mensagem
      lcd.setCursor(0, 1);                                        // Seto o cursor de escrita no display
      lcd.print(" AO BROKER...");                                 // Escrevo outra mensagem
      lcd.setCursor(15, 1);                                       // Seto o cursor de escrita no display
      delay(600);                                                 // Delay de 600ms
      lcd.clear();                                                // Limpo o LCD
    }
  }// End while(!client.connected) 
}//End reconectbroker

void PbSubSystem(){                                               // Função envia os dados ao broker MQTT
  reconectbroker();                                               // Chamo a função de reconexão ao broker
  sprintf(mensagem," SEND: %s", digitos);                         // junto o termo "SEND:" e a string digitos no array do payload
  lcd.clear();                                                    // Limpo o Lcd  
  lcd.setCursor(0, 0);                                            // Seto o cursor de escrita no display
  lcd.print("STRING ENVIADA:");                                   // Escrevo outra mensagem
  tone(pin, 2500, 300);                                           // Aciono o buzzer em 2.5KHz
  tone(pin, 2500, 300);                                           // Aciono o buzzer em 2.5KHz
  lcd.setCursor(0, 1);                                            // Seto o cursor de escrita no display
  lcd.print(mensagem);                                            // Escrevo no lcd o payload
  client.publish("ARGOS_IOT", mensagem);                          // Envio ao broker o tópico "ARGOS_IOT" e o payload
  delay(1000);                                                    // Travo em 1 segundo ( já que se trata de um prgrama simples )
  tone(pin, 1500, 300);                                           // Aciono o buzzer em 1.5KHz
  tone(pin, 800, 300);                                            //   ||       ||       800Hz
}// End PbSubSystem()

// Função inicial de stup do sistema ==================================================================

void setup(){ 
  lcd.init();                                    // Inicio o LCD   
  lcd.clear();                                   // Limpo o LCD
  lcd.createChar(1, customChar);                 // criando o caracter especial 1              
  lcd.backlight();                               // Ligo o backlight
  lcd.setCursor(0, 0);                           // Seto o cursor de escrita no display
  lcd.print("  DESAFIO ARGOS ");                 // Escrevo no display
  lcd.setCursor(0, 1);                           // Seto o cursor de escrita de outra mensagem no display
  lcd.print("    MUNDO JIX   ");                 // Escrevo outra mensagem
  tone(pin, 2000, 300);
 // tone(pin, 2000, 300);
  tone(pin, 3000, 300);
  delay(1000);                                   // Delay de 1 segundo para aparecer a mensagem
  lcd.clear();                                   // Limpo o Lcd  
  lcd.setCursor(2, 0);                           // Seto o cursor de escrita no display
  WiFi.begin(ssid, password);                    // Inicio o sistema de WiFi do Esp32
  tone(pin, 800, 200);
  while (WiFi.status() != WL_CONNECTED)          // Enquanto não foi conectado na rede...
  {    
    delay(500); // delay de 500 ms enquanto o sistema é conectado.                               
    lcd.print(" CONECTANDO... ");                // Escrevo outra mensagem
    
  }
  lcd.clear();                                   // Limpo o Lcd  
  lcd.setCursor(1, 0);                           // Seto o cursor de escrita no display
  lcd.print(" CONECTADO ");                      // Escrevo outra mensagem
  lcd.setCursor(1, 1);                           // Seto o cursor de escrita no display
  lcd.print("  A REDE! ");                       // Escrevo outra mensagem
  tone(pin, 1000, 300);
  tone(pin, 3000, 300);
  tone(pin, 4186, 300);
  delay(1000);                                   // delay de 700 milissegundos 
}// End setup

// Função operacional do sistema embarcado ===========================================================

void loop(){     
  switch(aux){                                    // Se a variável aux faz referência a cada caso abaixo...
    case 0:                                       // Se aux for 0...
     aux = estado_1();                            // chamo a função estado 1 e retorno o novo valor para aux
    break;                                        // saio do switch
    case 1:                                       // Se aux for 0...
     aux = estado_2();                            // chamo a função estado 1 e retorno o novo valor para aux
    break;                                        // saio do switch     
    case 2:                                       // Se aux for 0...
     aux = estado_3();                            // chamo a função estado 1 e retorno o novo valor para aux
    break;                                        // saio do switch
    default:                                      // Se aux for 0...
     aux = estado_4();                            // chamo a função estado 1 e retorno o novo valor para aux
    break;                                        // saio do switch
  }// End switch(aux)  
}// End loop 
