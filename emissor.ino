// Bibliotecas para o LoRa by Sandeep Mistry versão 0.8.0
#include <LoRa.h>
#include <SPI.h>
// Bibliotecas para OLED display by Adafruit
#include <Wire.h>
#include <Adafruit_GFX.h>     // versão 1.10.12
#include <Adafruit_SSD1306.h> // versão 2.4.6
// Biblioteca para o DHT by Adafruit versão 1.4.2
#include <DHT.h>

/* Define entradas e saídas*/
#define pinBotao        12
#define DHTPIN          17
#define buzzer          25
#define pinSensorDentro 36
#define pinSensorFora   38

/* Endereço I2C do display */
#define OLED_ADDR 0x3c

/* distancia, em pixels, de cada linha em relacao ao topo do display */
#define OLED_LINE1 0
#define OLED_LINE2 10
#define OLED_LINE3 20
#define OLED_LINE4 30
#define OLED_LINE5 40
#define OLED_LINE6 50

/* Configuração da resolucao do display (este modulo possui display 128x64) */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DHTTYPE DHT11   // DHT 11

/* Definicoes para comunicação com radio LoRa */
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18

#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 915MHz de frequencia */

/* Define o máximo de passageiros*/
#define MAX_PASSAGEIROS 30

/* Define entradas e saídas*/
#define pinBotao        12
#define DHTPIN          17
#define buzzer          25
#define pinSensorDentro 36
#define pinSensorFora   38

/*
   Variáveis e objetos globais
*/
/* objeto para comunicação com sensor DHT22 */
DHT dht(DHTPIN, DHTTYPE);

/* objeto do display */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, 16);

/* variáveis que armazenam os valores máximo e mínimo de temperatura registrados. */
float    temperatura_max;
float    temperatura_min;

/* Define variáveis que armazenam valores globais */
bool in  = 0;
bool out = 0;
int  pessoa;

/* typedefs */
typedef struct __attribute__((__packed__))
{
  float         temperatura;
  float         umidade;
  float         temperatura_min;
  float         temperatura_max;
  int           pessoa;
} TDadosLora;

/*------------------------------------------------------------------------------------------*/
/*---------------------------------------FUNÇÕES--------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

int  atualiza_pessoas();
void atualiza_temperatura_max_e_minima(float temp_lida);
void escreve_pessoa_temperatura_umidade_display(float temp_lida, float umid_lida, int pess_lida);
void envia_medicoes_para_serial(float temp_lida, float umid_lida, int pess_lida);
void envia_informacoes_lora(float temp_lida, float umid_lida, int pess_lida);
bool init_comunicacao_lora(void);

/*------------------------------------------------------------------------------------------*/
/*---------------------------------------SETUP----------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(115200);

  pinMode(pinSensorDentro, INPUT);
  pinMode(pinSensorFora,   INPUT);
  pinMode(pinBotao,        INPUT);
  pinMode(buzzer,          OUTPUT);

  /* inicializa temperaturas dht */
  dht.begin();

  /* inicializa temperaturas máxima e mínima com a leitura inicial do sensor */
  temperatura_max = dht.readTemperature();
  temperatura_min = temperatura_max;

  /* inicializa display OLED */
  Wire.begin(4, 15);

  /* verifica se a inicialização do OLED foi feita*/
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
    Serial.println("Display OLED: falha ao inicializar");
  else
  {
    Serial.println("Display OLED: inicializacao ok");

    /* Limpa display e configura tamanho de fonte */
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, OLED_LINE1);
    display.println("Emissor. Inicializando...");
    display.display();
  }

  /* Tenta, até obter sucesso, comunicacao com o chip LoRa */
  while (init_comunicacao_lora() == false);


}

/*------------------------------------------------------------------------------------------*/
/*---------------------------------------LOOP-----------------------------------------------*/
/*------------------------------------------------------------------------------------------*/


void loop() {
  TDadosLora dados_lora;

  float temperatura_lida;
  float umidade_lida;

  /* Faz a leitura de temperatura e umidade do sensor */
  temperatura_lida = dht.readTemperature();
  umidade_lida     = dht.readHumidity();

  if ( isnan(temperatura_lida) || isnan(umidade_lida) )
    Serial.println("Erro ao ler sensor DHT11!");
  else
  {
    atualiza_pessoas();
    atualiza_temperatura_max_e_minima(temperatura_lida);
    envia_medicoes_para_serial(temperatura_lida, umidade_lida, pessoa);
    escreve_pessoa_temperatura_umidade_display(temperatura_lida, umidade_lida, pessoa);
    envia_informacoes_lora(temperatura_lida, umidade_lida, pessoa);
  }
  delay(500);
}

/*------------------------------------------------------------------------------------------*/
/*------------------------------------ATUALIZA PESSOAS--------------------------------------*/
/*------------------------------------------------------------------------------------------*/

int atualiza_pessoas()
{
  TDadosLora dados_lora;
  dados_lora.pessoa = pessoa;

  if (digitalRead(pinBotao)) {
    pessoa = 0;
    digitalWrite(buzzer, HIGH);
    delay(100);
  } else {
    digitalWrite(buzzer, LOW);
    delay(100);
  }

  if (!digitalRead(pinSensorDentro)) {
    in = 0;
  }

  if (digitalRead(pinSensorDentro) && !in) {
    in = 1;
    pessoa++;
  }

  if (!digitalRead(pinSensorFora)) {
    out = 0;
  }

  if (digitalRead(pinSensorFora) && !out) {
    out = 1;
    pessoa--;
  }

  if (pessoa <= 0) {
    pessoa = 0;
  }

  if (pessoa >= MAX_PASSAGEIROS) {
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
  } else {
    digitalWrite(buzzer, LOW);
  }
}
/*------------------------------------------------------------------------------------------*/
/*------------------------------------ENVIA LORA--------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void envia_informacoes_lora(float temp_lida, float umid_lida, int pess_lida)
{
  TDadosLora dados_lora;

  dados_lora.pessoa          = pess_lida;
  dados_lora.temperatura     = temp_lida;
  dados_lora.umidade         = umid_lida;
  dados_lora.temperatura_min = temperatura_min;
  dados_lora.temperatura_max = temperatura_max;

  LoRa.beginPacket();
  LoRa.write((unsigned char *)&dados_lora, 
              sizeof(TDadosLora));
  LoRa.endPacket();

}

/*------------------------------------------------------------------------------------------*/
/*--------------------------------------ENVIA SERIAL----------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void envia_medicoes_para_serial(float temp_lida, float umid_lida, int pess_lida)
{
  char mensagem[200];
  char i;

  for (i = 0; i < 5; i++)
    Serial.println(" ");

  if (digitalRead(pinBotao)) {
    Serial.println("Pessoas resetadas!");
  }

  /* constrói mensagens e as envia */
  Serial.println("------------------");

  memset(mensagem, 0, sizeof(mensagem));
  sprintf(mensagem, "- Porta 36(entra): %d", 
          digitalRead(pinSensorDentro));
  Serial.println(mensagem);

  memset(mensagem, 0, sizeof(mensagem));
  sprintf(mensagem, "- Porta 38(sai): %d", 
          digitalRead(pinSensorFora));
  Serial.println(mensagem);

  /* - quantidade de pessoas */
  memset(mensagem, 0,  sizeof(mensagem));
  sprintf(mensagem, "- Pessoas: %d", 
          pess_lida);
  Serial.println(mensagem);

  /* - temperatura atual */
  memset(mensagem, 0, sizeof(mensagem));
  sprintf(mensagem, "- Temp: %.2f C", 
          temp_lida);
  Serial.println(mensagem);

  //- umidade relativa do ar atual
  memset(mensagem, 0, sizeof(mensagem));
  sprintf(mensagem, "- Umid: %.2f/100", 
          umid_lida);
  Serial.println(mensagem);

  //- temperatura maxima
  memset(mensagem, 0, sizeof(mensagem));
  sprintf(mensagem, "- Temp Max: %.2f C", 
          temperatura_max);
  Serial.println(mensagem);

  //- temperatura minima
  memset(mensagem, 0, sizeof(mensagem));
  sprintf(mensagem, "- Temp Min: %.2f C", 
          temperatura_min);
  Serial.println(mensagem);

  Serial.println("------------------");
}

/*------------------------------------------------------------------------------------------*/
/*------------------------------------ESCREVE DISPLAY---------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void escreve_pessoa_temperatura_umidade_display(float temp_lida, float umid_lida, int pess_lida)
{
  TDadosLora dados_lora;

  char str_pess[10]         = {0};
  char str_temp[10]         = {0};
  char str_umid[10]         = {0};
  char str_temp_max_min[20] = {0};

  /* formata para o display 
  as strings de temperatura e umidade */
  sprintf(str_pess, "%d", pess_lida);
  sprintf(str_temp, "%.2fC", temp_lida);
  sprintf(str_umid, "%.2f/100", umid_lida);
  sprintf(str_temp_max_min, "%.2fC / %.2fC", 
          temperatura_min, temperatura_max);

  display.clearDisplay();

  display.setCursor(0, OLED_LINE1);
  display.println("INFORMACOES");

  display.setCursor(0, OLED_LINE2);
  display.print("Pessoas: ");
  display.println(str_pess);

  display.setCursor(0, OLED_LINE3);
  display.print("Temp: ");
  display.println(str_temp);

  display.setCursor(0, OLED_LINE4);
  display.print("Umidade: ");
  display.println(str_umid);

  display.setCursor(0, OLED_LINE5);
  display.println("Temp min/max: ");
  display.println(str_temp_max_min);

  display.display();
}

/*------------------------------------------------------------------------------------------*/
/*------------------------------------INICIA LORA-------------------------------------------*/
/*------------------------------------------------------------------------------------------*/

bool init_comunicacao_lora(void)
{
  bool status_init = false;
  Serial.println("[LoRa Sender]" 
  "Tentando iniciar comunicacao"
  "com o radio LoRa...");
  SPI.begin(SCK_LORA, MISO_LORA,
            MOSI_LORA, SS_PIN_LORA);
  LoRa.setPins(SS_PIN_LORA,
               RESET_PIN_LORA,
               LORA_DEFAULT_DIO0_PIN);

  if (!LoRa.begin(BAND))
  {
    Serial.println("[LoRa Sender]"
    "Comunicacao com o radio LoRa"
    "falhou. Nova tentativa em 1 segundo...");
    delay(1000);
    status_init = false;
  }
  else
  {
    /* Configura o ganho do 
    receptor LoRa para 20dBm,
    o maior ganho possível */
    LoRa.setTxPower(HIGH_GAIN_LORA);
    Serial.println("[LoRa Sender]"
    "Comunicacao com o radio LoRa ok");
    status_init = true;
  }

  return status_init;
}

/*------------------------------------------------------------------------------------------*/
/*--------------------------------TEMPERATURA MAX E MIN-------------------------------------*/
/*------------------------------------------------------------------------------------------*/

void atualiza_temperatura_max_e_minima(float temp_lida)
{
  if (temp_lida > temperatura_max)
    temperatura_max = temp_lida;

  if (temp_lida < temperatura_min)
    temperatura_min = temp_lida;
}

/*------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------*/
