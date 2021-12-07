// Bibliotecas para o LoRa by Sandeep Mistry versão 0.8.0
#include <LoRa.h>
#include <SPI.h>
// Bibliotecas para OLED display by Adafruit
#include <Wire.h>
#include <Adafruit_GFX.h>         // versão 1.10.12
#include <Adafruit_SSD1306.h>     // versão 2.4.6
// Biblioteca para WiFi
#include <WiFi.h>
// Bibliotecas para o Firebase 
#include <IOXhop_FirebaseESP32.h> // https://github.com/artronshop/IOXhop_FirebaseESP32
#include <ArduinoJson.h>          // URL adicional para esta bilioteca: https://dl.espressif.com/dl/package_esp32_index.json --> by Benoit Blanchon versão 5.13.5

/* Comunicacao com o WiFi e Firebase*/
#define WIFI_SSID     "NOME DO WIFI"      
#define WIFI_PASSWORD "SENHA DO WIFI"     
#define FIREBASE_HOST "NOME DO FIREBASE"  
#define FIREBASE_AUTH "SENHA DO FIREABSE" 

/* Definicoes para comunicação com radio LoRa */
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18

#define HIGH_GAIN_LORA     20     /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */

/* Definicoes do OLED */
#define OLED_SDA_PIN    4
#define OLED_SCL_PIN    15
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_ADDR       0x3C
#define OLED_RESET      16

/* Offset de linhas no display OLED */
#define OLED_LINE1     0
#define OLED_LINE2     10
#define OLED_LINE3     20
#define OLED_LINE4     30
#define OLED_LINE5     40
#define OLED_LINE6     50

/* Definicoes gerais */
#define DEBUG_SERIAL_BAUDRATE    115200

/* Variaveis e objetos globais */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* Local prototypes */
void display_init(void);
bool init_comunicacao_lora(void);

/* typedefs */
typedef struct __attribute__((__packed__))
{
  float        temperatura;
  float        umidade;
  float        temperatura_min;
  float        temperatura_max;
  int          pessoa;
} TDadosLora;

WiFiServer server(80);

/*------------------------------------------------------------------------------------------*/
/*---------------------------------------SETUP----------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void setup()
{
  /*-----------------------------------------*/
  /*------------Inicializa WiFi--------------*/

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                       //inicia comunicação com wifi com rede definida anteriormente

  Serial.print("Conectando ao wifi");                         //imprime "Conectando ao wifi"
  while (WiFi.status() != WL_CONNECTED)                       //enquanto se conecta ao wifi fica colocando pontos
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();                                           //imprime pulo de linha

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado ao WiFi >>> ");                   //imprime em qual WiFi está conectado
    Serial.println(WIFI_SSID);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);               //inicia comunicação com firebase definido anteriormente

  /*-----------------------------------------*/
  /*------------Inicializa LoRa--------------*/

  /* Configuracao da I²C para o display OLED */
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

  /* Display init */
  display_init();

  /* Print message telling to wait */
  display.clearDisplay();
  display.setCursor(0, OLED_LINE1);
  display.print("Receptor ativo. Aguarde...");
  display.display();

  Serial.begin(DEBUG_SERIAL_BAUDRATE);
  while (!Serial);

  /* Tenta, até obter sucesso, comunicacao com o chip LoRa */
  while (init_comunicacao_lora() == false);

}

/*------------------------------------------------------------------------------------------*/
/*---------------------------------------LOOP-----------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void loop()
{
  char byte_recebido;
  int packet_size = 0;
  int lora_rssi = 0;
  TDadosLora dados_lora;
  char * ptInformaraoRecebida = NULL;

  /* Verifica se chegou alguma 
  informação do tamanho esperado */
  packet_size = LoRa.parsePacket();

  if (packet_size == sizeof(TDadosLora))
  {
    Serial.println("[LoRa Receiver]"
    "Há dados a serem lidos");

    /* Recebe os dados conforme protocolo */
    ptInformaraoRecebida = (char *)&dados_lora;
    while (LoRa.available())
    {
      byte_recebido = (char)LoRa.read();
      *ptInformaraoRecebida = byte_recebido;
      ptInformaraoRecebida++;
    }

    /* Escreve RSSI de recepção e informação recebida */
    display.clearDisplay();

    display.setCursor(0, OLED_LINE1);
    display.print("RSSI: ");
    display.print(LoRa.packetRssi());
    display.println(" dBm");

    display.setCursor(0, OLED_LINE2);
    display.print("Pessoas: ");
    display.println(dados_lora.pessoa);

    display.setCursor(0, OLED_LINE3);
    display.print("Temp: ");
    display.println(dados_lora.temperatura);

    display.setCursor(0, OLED_LINE4);
    display.print("Umid: ");
    display.println(dados_lora.umidade);

    display.setCursor(0, OLED_LINE5);
    display.print("T Min/Max: ");
    display.setCursor(0, OLED_LINE6);
    display.print(dados_lora.temperatura_min);
    display.print("/");
    display.println(dados_lora.temperatura_max);

    display.display();
  }

  Firebase.setInt("/Linha_501_3/numeroPassageiros", dados_lora.pessoa);
  Firebase.setFloat("/Linha_501_3/temperatura", dados_lora.temperatura);
  Firebase.setFloat("/Linha_501_3/umidade", dados_lora.umidade);

  //  Firebase.setInt("/Linha_501_4/numeroPassageiros", dados_lora.pessoa);
  //  Firebase.setFloat("/Linha_501_4/temperatura", dados_lora.temperatura);
  //  Firebase.setFloat("/Linha_501_4/umidade", dados_lora.umidade);
  delay(500);

}

/*------------------------------------------------------------------------------------------*/
/*------------------------------------INICIA DISPLAY----------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void display_init(void)
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("[LoRa Receiver] Falha ao inicializar comunicacao com OLED");
  }
  else
  {
    Serial.println("[LoRa Receiver] Comunicacao com OLED inicializada com sucesso");

    /* Limpa display e configura tamanho de fonte */
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
  }
}

/*------------------------------------------------------------------------------------------*/
/*------------------------------------INICIA LORA-------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

bool init_comunicacao_lora(void)
{
  bool status_init = false;
  Serial.println("[LoRa Receiver] Tentando iniciar comunicacao com o radio LoRa...");
  SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
  LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);

  if (!LoRa.begin(BAND))
  {
    Serial.println("[LoRa Receiver] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");
    delay(1000);
    status_init = false;
  }
  else
  {
    /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */
    LoRa.setTxPower(HIGH_GAIN_LORA);
    Serial.println("[LoRa Receiver] Comunicacao com o radio LoRa ok");
    status_init = true;
  }

  return status_init;
}
