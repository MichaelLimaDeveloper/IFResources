// Define macros for input and output pin etc.
#include "PinDefinitionsAndMore.h"

//Bibliotecas
#include "WiFi.h"
#include "PubSubClient.h"
#include "ESPmDNS.h"
#include "DHTesp.h"
#include "heltec.h"
#include "IRremote.hpp"
 
//Net Setup
#define NET_SSID "Michael"
#define NET_PASSWORD "1234567890"


//MQTT Setup
#define MQTT_ID "ifresources-tcc-lora32"
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_MILLIS_TOPIC "tcc_ifresources_millis"
#define MQTT_SECONDS_TOPIC "tcc_ifresources_seconds"
#define MQTT_DHT_TEMP_TOPIC "tcc_ifresources_temp"
#define MQTT_DHT_HUMD_TOPIC "tcc_ifresources_humd"
#define MQTT_NIVEL_TOPIC "tcc_ifresources_nivel"
#define MQTT_SENDRAW_TOPIC "tcc_ifresources_sendraw_manage"

// Define net client
WiFiClient espClient; 

// Define mqtt client
PubSubClient MQTT(espClient);

//Variáveis que armazenam as informações que são enviadas ao broker
char millis_str[10] = "";
char seconds_str[10] = "";
char temp_str[10] = "";
char humd_str[10] = "";
char nivel_str[10] = "";

//Definição da Tela e Sensor DHT22
#define BAND    915E6  //Escolha a frequência
DHTesp dht;

//Variavéis do Nivel d'água
using namespace std;
bool isPressed = "";
int nivel = 0;
int buttonState = 0;

//Variavéis do Sensor DHT22
float currentTemp;
float currentHumidity;

// Variável global para armazenar o tempo inicial
unsigned long tempoInicial;

//Protótipo da função
void getTemp();
void getHumidity();
void getNivel();
void sendPacket();

void setupWifi(){
  if(WiFi.status() == WL_CONNECTED){
    return;
  } else {
    // Connect with WiFi

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(NET_SSID);
  
    WiFi.begin(NET_SSID, NET_PASSWORD);

    // Loop para checar a conexão

    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    // Display connection info

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP Address:");
    Serial.println(WiFi.localIP());
    }
}

void setupMQTT(){
    //Config MQTT Broker connection

    MQTT.setServer(MQTT_BROKER, MQTT_PORT);
    MQTT.setCallback(mqtt_ifresources_callback); 

    //Executar conexão

    while (!MQTT.connected()){
        Serial.print("- MQTT Setup: Tentando se conectar ao Broker MQTT: ");
        Serial.println(MQTT_BROKER);

        if(MQTT.connect(MQTT_ID)){
            Serial.println("- MQTT Setup: Conectado com sucesso");
             MQTT.subscribe(MQTT_SENDRAW_TOPIC);
        } else {
            Serial.println("- MQTT Setup: Falha ao se conectar, tentando novamente em 2s");
            delay(2000);
        }
    }
}

//Função que ler a temperatura e atibiu a variável currentTemp.
void getTemp()
{

  //Atribuindo a variavel float a função
  float temperature = dht.getTemperature();

  //Verifica se houve mudança na temperatura
  if (temperature != currentTemp) {
    currentTemp = temperature;
    
    // Liga o LED
    digitalWrite(LED, HIGH);
    
    // Espera 500 milissegundos
    delay(500);
    
    // Desliiga o LED
    digitalWrite(LED, LOW);
    
    // Espera 500 milissegundos
    delay(500);
  }
  delay(1000);
}

//Função que ler a Umidade e atibiu a variável currentHumidity.
void getHumidity()
{
  //Atribuindo a variavel float a função
  float humidity = dht.getHumidity();

  //Verifica se houve mudança na temperatura
  if (humidity != currentHumidity) {
    currentHumidity = humidity;
    
    // Liga o LED
    digitalWrite(LED, HIGH);
    
    // Espera 500 milissegundos
    delay(500);
    
    // Desliiga o LED
    digitalWrite(LED, LOW);
    
    // Espera 500 milissegundos
    delay(500);                
  }
  delay(1000);
}

//Função referente ao funcionamento do nivel d'agua
void getNivel()
{

    if (digitalRead(36) == 1)
    {
        nivel = 4;
    }
    
    else if (digitalRead(37) == 1)
    {
        nivel = 3;
    }

    else if (digitalRead(38) == 1)
    {
        nivel = 2;
    }

    else if (digitalRead(39) == 1)
    {
        nivel = 1;
    }
    else
    {
        nivel = 0;
    }

}

//O baudrate no monitor serial é (115200)
void setup(void)
{

    // Armazene o tempo inicial em milissegundos
    tempoInicial = millis();
    
    //inicializa o LED
    pinMode(LED, OUTPUT);
  
    //Habilita diversas Configurações do Lora32
    Heltec.begin(true, true, true , true , BAND);
    /*(
      true = Habilita o Display, 
      true = Heltec.Heltec.Heltec.LoRa Disable, 
      true = Habilita debug Serial, 
      true = Habilita o PABOOST, 
      BAND = Frequência BAND.
    )*/
  
    //Primeira Tela: Exibe os criadores do projeto enquanto de conecta ao Wi-Fi e ao Broker MQTT
    Heltec.display->init();
    Heltec.display->setContrast(255);
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_24);
    Heltec.display->drawString(0, 0, "IFresources");
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(0, 25, "@ifrjniteroi");
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 45, "Conectando ao Wi-Fi...");
    Heltec.display->display();
    delay(1000);
  
    //inicializa o DHT no pino 25
    dht.setup(25, DHTesp::DHT22); 
  
    //Atribuindo as variaveis as funções
    currentTemp = dht.getTemperature();
    currentHumidity = dht.getHumidity();

    //Pinos referentes ao nivel d'água
    pinMode(36, INPUT);
    pinMode(37, INPUT);
    pinMode(38, INPUT);
    pinMode(39, INPUT);

    // Call setup wifi
    setupWifi();
    // Call setup mqtt
    setupMQTT();
}

void loop(void)
{
  //Funções de Leitura de Temperatura e Umidade
  getTemp();
  getHumidity();
  getNivel();

  // Converte a função millis para uma função que exibe os segundos
  unsigned long tempoAtual = millis();
  unsigned long segundos = (tempoAtual - tempoInicial) / 1000;

  //Segunda Tela: Exibe os valores de temperatura na tela do Lora32.
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(30, 5, "Temperatura"); //"Enviando"
  Heltec.display->drawString(33, 30, (String)currentTemp);
  Heltec.display->drawString(78, 30, "°C");
  Heltec.display->display();

  //Monitor Serial
  Serial.println("Tempo em Milissegundos: " + (String)millis());
  Serial.println("Tempo em Segundos: " + (String)segundos);  
  Serial.println("Temperatura: " + (String)currentTemp + "°C");
  Serial.println("Umidade: " + (String)currentHumidity + "%");
  Serial.println("Nivel da Água: " + (String)nivel);
  Serial.println("---");

  //Envia Milissegundos
  sprintf(millis_str, "%d", millis());
  MQTT.publish(MQTT_MILLIS_TOPIC, millis_str);

  //Envia Segundos
  sprintf(seconds_str, "%s", (String)segundos);
  MQTT.publish(MQTT_SECONDS_TOPIC, seconds_str);

  //Envia Temperatura
  sprintf(temp_str, "%s", (String)currentTemp );
  MQTT.publish(MQTT_DHT_TEMP_TOPIC, temp_str);

  //Envia Umidade
  sprintf(humd_str, "%s", (String)currentHumidity );
  MQTT.publish(MQTT_DHT_HUMD_TOPIC, humd_str);

  //Envia o nivel da água
  sprintf(nivel_str, "%s", (String)nivel);
  MQTT.publish(MQTT_NIVEL_TOPIC, nivel_str);

  //Configuração de Conexões e Configuração de Delay
  setupWifi();
  setupMQTT();
  MQTT.loop();
  delay(1000);
}

// Callback function
// Called when data is received in one of topics
void mqtt_ifresources_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;
  Serial.print("- MQTT Callback Topic: ");
  Serial.println(topic);

  //obtem a string do payload recebido
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }

//Controlando Led Infravermelho
  
  //Ligar
  if (msg.equals("P1"))
    {
      #if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
  
      const uint16_t rawData[] = {3064, 1624, 452, 1128, 452, 1124, 456, 360, 456, 360, 456, 360, 456, 1124, 456, 360, 452, 364, 452, 1128, 456, 1124, 456, 360, 456, 1124, 456, 360, 452, 364, 452, 1124, 456, 1124, 456, 360, 456, 1124, 456, 1124, 456, 360, 456, 360, 456, 1124, 452, 364, 456, 360, 456, 360, 452, 1128, 456, 360, 456, 360, 452, 360, 460, 360, 452, 364, 452, 360, 456, 360, 452, 364, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 360, 456, 1124, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 364, 452, 360, 456, 364, 452, 360, 456, 360, 452, 364, 452, 364, 452, 364, 456, 360, 452, 364, 452, 364, 456, 360, 452, 364, 456, 360, 452, 364, 452, 360, 452, 368, 452, 364, 452, 360, 456, 360, 452, 364, 452, 364, 452, 364, 456, 360, 456, 360, 452, 364, 456, 360, 456, 360, 456, 360, 452, 364, 452, 364, 456, 360, 456, 356, 456, 336, 480, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 1124, 456, 360, 456, 1124, 456, 360, 452, 364, 452, 1124, 456, 1128, 452, 360, 456, 4456, 3060, 1628, 452, 1124, 456, 1124, 456, 360, 456, 360, 456, 360, 456, 1124, 456, 360, 456, 360, 452, 1128, 456, 1124, 456, 360, 452, 1128, 452, 360, 456, 360, 456, 1124, 456, 1124, 456, 360, 452, 1128, 456, 1124, 452, 364, 452, 364, 452, 1124, 456, 360, 456, 360, 456, 1124, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 364, 452, 364, 452, 364, 456, 360, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 1124, 456, 360, 456, 360, 452, 1128, 456, 360, 456, 360, 456, 1124, 452, 1128, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 1124, 456, 1124, 456, 360, 456, 1124, 452, 364, 456, 360, 456, 360, 456, 360, 452, 1128, 456, 1124, 452, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 360, 452, 364, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 452, 364, 452, 1128, 452, 360, 456, 360, 456, 360, 456, 1124, 456, 360, 456, 1124, 456, 360, 456, 360, 456, 1124, 456, 360, 456, 1120, 460, 1124, 452}; // Using exact NEC timing
      IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  
      Serial.println(F("Send NEC 16 bit address=0xFB04 and command 0x08 with exact timing (16 bit array format)"));
      Serial.flush();
      delay(1000);
      
        
      Serial.println("- MQTT Sub Conn: (P1) Ligar Ar-condicionado Received");

      #endif  
    }

   //Desligar
   else if (msg.equals("P0"))
    {
      #if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
  
      const uint16_t rawData[] = {3064, 1620, 456, 1124, 456, 1124, 456, 360, 456, 360, 456, 360, 456, 1124, 456, 360, 452, 364, 452, 1124, 460, 1124, 452, 360, 456, 1124, 456, 360, 456, 364, 452, 1124, 456, 1124, 456, 360, 452, 1128, 456, 1124, 452, 364, 452, 364, 456, 1124, 452, 360, 460, 356, 460, 356, 456, 1124, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 360, 460, 360, 456, 356, 456, 360, 456, 1124, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 452, 364, 456, 360, 456, 360, 456, 360, 456, 356, 456, 364, 452, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 456, 360, 452, 360, 456, 364, 456, 356, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 364, 456, 360, 452, 364, 452, 364, 452, 360, 456, 360, 456, 364, 452, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 456, 360, 452, 364, 452, 364, 456, 356, 456, 364, 456, 1120, 456, 360, 456, 1124, 456, 360, 456, 360, 456, 1124, 456, 1124, 456, 360, 456, 3972, 3088, 1596, 484, 1096, 480, 1100, 480, 336, 480, 336, 452, 364, 480, 1100, 480, 336, 480, 336, 480, 1096, 484, 1096, 484, 332, 480, 1100, 480, 336, 480, 336, 480, 1100, 480, 1100, 480, 336, 480, 1096, 480, 1100, 480, 336, 480, 336, 484, 1096, 480, 336, 480, 336, 480, 1100, 480, 336, 480, 336, 480, 332, 484, 332, 484, 332, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 484, 332, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 476, 1100, 480, 336, 484, 332, 480, 1100, 452, 1128, 480, 336, 480, 336, 476, 340, 480, 336, 480, 332, 484, 332, 484, 1096, 484, 1096, 480, 336, 480, 1100, 480, 336, 480, 336, 480, 336, 452, 364, 480, 1100, 476, 1104, 480, 336, 480, 336, 476, 340, 476, 336, 480, 336, 484, 332, 480, 336, 456, 360, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 452, 364, 480, 336, 476, 340, 480, 332, 484, 336, 476, 336, 480, 336, 480, 336, 480, 336, 480, 336, 480, 336, 452, 364, 480, 336, 476, 1104, 452, 364, 452, 364, 452, 364, 452, 1124, 456, 360, 452, 1128, 452, 1128, 452, 1124, 456, 360, 456, 364, 452, 1124, 452, 1128, 452}; // Using exact NEC timing
      IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  
      Serial.println(F("Send NEC 16 bit address=0xFB04 and command 0x08 with exact timing (16 bit array format)"));
      Serial.flush();
      delay(1000);
        
      Serial.println("- MQTT Sub Conn: (P0) Desligar Ar-condicionado Received");

      #endif
    }

    //Aumentar
    else if (msg.equals("A1"))
    {
      #if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
  
      const uint16_t rawData[] = {3112, 1572, 456, 1124, 508, 1072, 504, 312, 456, 360, 504, 312, 504, 1076, 504, 312, 452, 364, 452, 1128, 504, 1072, 508, 312, 452, 1124, 508, 308, 456, 360, 508, 1072, 508, 1072, 456, 360, 504, 1076, 456, 1124, 504, 312, 452, 364, 452, 1128, 452, 364, 504, 312, 504, 312, 452, 1124, 456, 360, 456, 364, 452, 364, 452, 360, 456, 360, 508, 308, 508, 308, 456, 360, 508, 308, 456, 360, 452, 364, 456, 360, 508, 308, 452, 364, 456, 360, 504, 312, 504, 312, 504, 312, 452, 364, 452, 364, 452, 1124, 508, 312, 504, 312, 452, 364, 504, 312, 452, 360, 508, 308, 508, 308, 508, 308, 508, 308, 456, 360, 456, 360, 508, 308, 456, 360, 456, 360, 456, 364, 452, 360, 504, 312, 452, 364, 452, 364, 504, 312, 504, 312, 508, 308, 452, 364, 504, 312, 504, 312, 452, 364, 452, 364, 504, 312, 452, 360, 508, 308, 508, 308, 456, 360, 508, 308, 508, 308, 508, 308, 456, 360, 456, 360, 504, 312, 508, 308, 508, 308, 456, 360, 452, 364, 504, 312, 452, 364, 504, 312, 456, 360, 452, 364, 452, 364, 504, 312, 504, 312, 504, 312, 452, 360, 456, 364, 504, 308, 456, 360, 456, 360, 508, 308, 456, 1124, 456, 360, 504, 1076, 456, 360, 456, 360, 456, 1124, 452, 1128, 504, 312, 504, 3756, 3060, 1624, 460, 1120, 456, 1124, 508, 308, 456, 336, 480, 360, 456, 1124, 456, 360, 504, 312, 456, 1124, 456, 1124, 452, 364, 452, 1128, 504, 312, 452, 364, 452, 1128, 452, 1124, 456, 364, 452, 1124, 456, 1124, 456, 360, 456, 360, 456, 1124, 456, 360, 504, 312, 452, 1128, 504, 312, 504, 312, 452, 364, 452, 364, 452, 364, 504, 312, 452, 364, 452, 364, 452, 360, 456, 364, 452, 360, 508, 308, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 1128, 452, 364, 456, 360, 456, 1124, 456, 360, 452, 364, 452, 1128, 452, 1128, 452, 364, 452, 364, 452, 364, 460, 352, 456, 360, 456, 360, 456, 1124, 508, 1072, 456, 360, 452, 1128, 456, 360, 452, 364, 456, 360, 452, 364, 452, 1128, 452, 1128, 452, 360, 456, 364, 452, 360, 456, 360, 508, 308, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 340, 476, 364, 504, 312, 452, 364, 452, 364, 456, 360, 452, 364, 452, 364, 452, 360, 456, 360, 456, 364, 452, 360, 456, 360, 456, 360, 456, 1124, 456, 360, 456, 1124, 508, 308, 456, 1124, 452, 364, 456, 1124, 452, 364, 452, 364, 452, 1124, 456, 1124, 456, 1124, 456, 1124, 456}; // Using exact NEC timing
      IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  
      Serial.println(F("Send NEC 16 bit address=0xFB04 and command 0x08 with exact timing (16 bit array format)"));
      Serial.flush();
      delay(1000);
      
        
      Serial.println("- MQTT Sub Conn: (A1) Diminuir Temperatura Received");

      #endif  
    } 
    
    //Diminuir
    else if (msg.equals("D1"))
    {
      #if !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
  
      const uint16_t rawData[] = {3060, 1624, 456, 1124, 452, 1128, 452, 364, 452, 364, 452, 364, 452, 1128, 480, 332, 484, 336, 452, 1124, 456, 1124, 456, 360, 456, 1124, 456, 360, 452, 364, 456, 1124, 452, 1128, 452, 364, 452, 1128, 452, 1124, 456, 360, 456, 364, 452, 1124, 456, 360, 456, 360, 480, 336, 456, 1124, 480, 336, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 364, 452, 360, 484, 332, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 1124, 452, 364, 452, 364, 480, 336, 452, 364, 452, 364, 452, 360, 456, 364, 452, 360, 456, 364, 452, 360, 456, 364, 452, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 480, 336, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 360, 484, 332, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 452, 1128, 452, 364, 452, 1128, 452, 360, 456, 360, 484, 1096, 456, 1124, 456, 360, 456, 4628, 3112, 1572, 456, 1124, 452, 1128, 456, 360, 452, 364, 504, 312, 456, 1124, 504, 312, 452, 364, 452, 1128, 452, 1128, 452, 360, 456, 1128, 500, 312, 456, 364, 452, 1124, 456, 1124, 452, 364, 452, 1128, 504, 1076, 504, 312, 452, 364, 456, 1124, 452, 364, 452, 364, 452, 1124, 456, 360, 508, 308, 456, 360, 504, 312, 504, 312, 508, 308, 504, 312, 452, 364, 504, 312, 456, 360, 504, 312, 456, 360, 504, 312, 504, 312, 452, 364, 452, 364, 452, 364, 452, 1128, 504, 308, 508, 308, 456, 1124, 456, 360, 504, 312, 456, 1124, 504, 1076, 452, 364, 452, 364, 452, 364, 456, 360, 452, 368, 448, 364, 452, 1124, 508, 1076, 452, 360, 508, 1072, 456, 360, 504, 312, 456, 360, 508, 308, 456, 1124, 456, 1124, 452, 364, 452, 364, 504, 312, 452, 364, 452, 364, 452, 364, 452, 364, 452, 364, 452, 360, 456, 364, 504, 308, 508, 312, 452, 360, 504, 312, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 456, 360, 452, 364, 456, 360, 452, 364, 452, 364, 504, 312, 504, 312, 452, 364, 452, 364, 452, 360, 456, 364, 452, 364, 452, 360, 456, 1124, 508, 308, 456, 360, 456, 360, 504, 1076, 452, 364, 504, 1076, 456, 360, 452, 364, 452, 1128, 452, 360, 456, 1124, 456, 1124, 456}; // Using exact NEC timing
      IrSender.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  
      Serial.println(F("Send NEC 16 bit address=0xFB04 and command 0x08 with exact timing (16 bit array format)"));
      Serial.flush();
      delay(1000);
      
      
      Serial.println("- MQTT Sub Conn: (D1) Diminuir Temperatura Received");

      #endif
    }
}