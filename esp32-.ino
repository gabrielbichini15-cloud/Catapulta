#include <BluetoothSerial.h> // Biblioteca nativa do ESP32
#include <Stepper.h>
#include <math.h>

// Verifica se o Bluetooth está habilitado corretamente
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// ======================================================
// BLUETOOTH (Nativo do ESP32)
// ======================================================
BluetoothSerial SerialBT; 

// ======================================================
// CONFIGURAÇÕES DOS MOTORES
// ======================================================
const int passosPorVolta = 2048;

// Pinos sugeridos para ESP32 (Evite pinos de apenas entrada como 34-39)
// Motor 1: IN1=13, IN2=12, IN3=14, IN4=27
Stepper motor1(passosPorVolta, 13, 14, 12, 27);

// Motor 2: IN1=26, IN2=25, IN3=33, IN4=32
Stepper motor2(passosPorVolta, 26, 33, 25, 32);

// ======================================================
// CONFIGURAÇÕES GERAIS E FÍSICA
// ======================================================
const float passosPorGrau = 2048.0 / 360.0;
float anguloAtual = 0;
const float g = 9.81;
const float m = 0.020;
const float k = 25.0;
const float r = 0.08;

String entrada = "";

// ======================================================
// BOTÃO LIGA/DESLIGA
// ======================================================
const int botaoPin = 4;

bool sistemaLigado = false;
bool ultimoEstadoBotao = HIGH;

unsigned long ultimoDebounce = 0;
const unsigned long debounceDelay = 50;

// ======================================================
// LED INDICADOR
// ======================================================
const int ledPin = 2;

void setup() {
  Serial.begin(115200);

  SerialBT.begin("Catapulta_ESP32");

  motor1.setSpeed(12);
  motor2.setSpeed(12);

  pinMode(botaoPin, INPUT_PULLUP);

  // LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println("Sistema iniciado.");
  Serial.println("Pressione o botão para ligar/desligar.");
}

void loop() {

  verificarBotao();

  // Se desligado, não faz nada
  if (!sistemaLigado) {
    return;
  }

  while (SerialBT.available()) {

    char c = SerialBT.read();

    if (c == ' ') continue;

    if (c == '\n' || c == '\r') {

      if (entrada.length() > 0) {

        float distancia = entrada.toFloat();

        Serial.print("Recebido via BT: ");
        Serial.println(distancia);

        entrada = "";

        if (distancia >= 0.5 && distancia <= 4.0) {

          prepararLancamento(distancia);

        } else {

          SerialBT.println("Distancia invalida (0.5 a 4m)");
        }
      }
    }

    else if ((c >= '0' && c <= '9') || c == '.') {

      entrada += c;
    }
  }
}

void prepararLancamento(float distancia) {
  // A lógica matemática permanece a mesma
  float phi = sqrt((distancia * m * g) / (r * r * k));
  float anguloDesejado = phi * 180.0 / PI;

  if (anguloDesejado > 120) anguloDesejado = 120;
  if (anguloDesejado < 5) anguloDesejado = 5;

  SerialBT.print("Angulo calculado: ");
  SerialBT.println(anguloDesejado);

  moverParaAngulo(anguloDesejado);
  SerialBT.println("Catapulta armada!");
}

void moverParaAngulo(float novoAngulo) {
  float diferenca = novoAngulo - anguloAtual;
  int passos = diferenca * passosPorGrau;

  if (passos > 0) {
    for (int i = 0; i < passos; i++) {
      motor1.step(1);
      motor2.step(1);
    }
  } else {
    for (int i = 0; i < abs(passos); i++) {
      motor1.step(-1);
      motor2.step(-1);
    }
  }
  anguloAtual = novoAngulo;
  pararMotores();
}

void pararMotores() {
  // Desliga os pinos novos definidos acima
  int pins[] = {13, 14, 12, 27, 26, 33, 25, 32};
  for(int i=0; i<8; i++) digitalWrite(pins[i], LOW);
}


void verificarBotao() {

  bool leitura = digitalRead(botaoPin);

  if (leitura != ultimoEstadoBotao) {
    ultimoDebounce = millis();
  }

  if ((millis() - ultimoDebounce) > debounceDelay) {

    static bool estadoAnterior = HIGH;

    if (leitura == LOW && estadoAnterior == HIGH) {

      sistemaLigado = !sistemaLigado;

      if (sistemaLigado) {

  digitalWrite(ledPin, HIGH);

  Serial.println("Sistema LIGADO");
  SerialBT.println("Sistema LIGADO");

} else {

  digitalWrite(ledPin, LOW);

  Serial.println("Sistema DESLIGADO");
  SerialBT.println("Sistema DESLIGADO");

  pararMotores();
}
    }

    estadoAnterior = leitura;
  }

  ultimoEstadoBotao = leitura;
}
