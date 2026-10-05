/*
  MoT LoRa Site Survey Versão Configura Rádio | WissTek IoT
  Última versão: Branquinho / Felipe / Anderson
  Hardware: PKLoRa ESP32
*/

// ESCOLHA DO HARDWARE PKLORA ESP32 OU NODEMCU

// Hardware - Configuração da Seleção do Tipo de Hardware (ESP32 ou NODEMCU)
// Remova o comentário da linha referente à placa que você está usando no momento e comente a outra

//#define PKLORA_ESP32 // HARDWARE COM ESP32
#define PKLORA_NODEMCU // HARDWARE COM ESP12E - NODEMCU

//=======================================================================
// 1 - Bibliotecas
#include "Bibliotecas.h"  // Arquivo contendo declaração de bibliotecas e variáveis
//=======================================================================

//#define loraCRC                // Habilita ou disabilita o uso CRC, por padrão o CRC não é usado.

//=======================================================================
// 2 - Variáveis
//=======================================================================

// Váriáveis utilizadas no código
int RSSI_dBm_DL; // Variável com a potência rádio recebida (RSSI) em dBm
int RSSI_DL;     // Variável de mapeamento da RSSI em um valor de 0 a 255 para colocar no pacote

float SNR_DL_bruto;   // Variável com a relação sinal ruído
uint8_t SNR_DL;           // Variável inteira para enviar a SNR, que será convertida para a SNR original no Python

// ============== CAMADA MAC
#define TAMANHO_PACOTE 20
byte PacoteDL[TAMANHO_PACOTE];
byte PacoteUL[TAMANHO_PACOTE];

// ============= CAMADA DE REDE
// Identificação do sensor e tamanho de pacote
int ID_sensor = 1; // Variável de iIdentificação do sensor que está no pacote de DL byte 8
int ID_gateway = 0;    // Variável com o ID_gateway que estará no pacote de DL byte 10

// ============== CAMADA DE TRANSPORTE
int contador_pkt_DL = 0; // Variável para o contador de pacotes de DL
int contador_pkt_UL = 0; // Variável para o contador de pacotes de UL
uint16_t contadorUL = 0;
uint16_t contadorDL = 0;
uint16_t contadorSS = 0;

int luminosidade; // Variável que vai receber o valor da luminosidade entre 0 e 4095 - ADC 12 bits
uint8_t feedback_led_amarelo = 0;

//=======================================================================
// ------- 3 - Setup de inicialização ---------
//=======================================================================
// Inicializa as camadas
void setup() {

  Serial.begin(115200);
  delay(20);
  Serial.println("--- Iniciando Nó Sensor LoRa ---");

  // --- Inicialização de I/O ---
  pinMode(LED_VERMELHO_PIN, OUTPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);

  #if defined(PKLORA_ESP32) 
    pinMode(BOTAO_PIN, INPUT); 
    pinMode(LED_AMARELO_PIN, OUTPUT); 
    // Configuração ADC para o LDR
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    digitalWrite(LED_AMARELO_PIN, LOW);
  #endif

  pinMode(LDR_PIN, INPUT);

  // Garante que os LEDs iniciem desligados
  digitalWrite(LED_VERMELHO_PIN, LOW);
  digitalWrite(LED_VERDE_PIN, LOW);

  // --- Inicialização Módulo RF95 (LoRa) ---
  #if defined(PKLORA_ESP32)  
    // 1. Remapeia e inicializa o barramento SPI com os pinos do seu Kit
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);
  #endif

  // 2. Informa à biblioteca LoRa os pinos de controle
  LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(FREQUENCY_IN_HZ)) {
    Serial.println("Erro ao iniciar módulo RFM95");
  }

  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
 
  Serial.println("LoRa Inicializado com Sucesso!");
  
  // Pisca o LED Verde para indicar inicialização bem-sucedida
  digitalWrite(LED_VERDE_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_VERDE_PIN, LOW);

  #ifdef loraCRC   // Habilitação do CRC do chip lora  (Configurado em bibliotecas.h)
    LoRa.enableCrc();
  #endif

} // FIM DO SETUP

//=======================================================================
//  ------------ 4 - Loop de repetição ------------
//=======================================================================
// A função loop irá executar repetidamente
void loop() {
    
  // --- Controle de timeout do Comando 4 ---
  // Executado a cada iteração do loop, independente de novo pacote chegar
  if (controle_ativo) {
    unsigned long tempo_limite_ms = (unsigned long)tempo_radio * 50UL * 1000UL; // 10x o valor recebido em MAC3_TEMPO
    millis_inicio_controle = millis();

//    if (millis() - millis_inicio_controle >= tempo_limite_ms) {
//      reset_para_setup_inicial(); // Timeout atingido → volta ao SETUP
//    }
  
  }

/*

  unsigned long tempo_standby_ms = 2UL * time_out_lora_dl; // 2 min. sem Pacotes DL sobe para MAX  

  if (millis() - millis_standby_controle >= tempo_standby_ms) {
    Serial.println("TEMPO SEM RECEBER PACOTES - Time-Out");
    Serial.println("Voltando a Configuração LoRa BDC");

    millis_standby_controle = millis();
    reset_para_setup_inicial(); // Timeout atingido → volta ao SETUP
  }  

  if ((confirma_novo_radio_base != 4) & (confirma_novo_radio_base != 5)){ 
    millis_contador_DL = millis();
  }

*/

  Phy_radio_receive_DL(); // Função que recebe os pacotes pelo rádio

}
