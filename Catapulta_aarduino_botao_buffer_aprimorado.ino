#include <SoftwareSerial.h>
#include <Stepper.h>
#include <math.h>

// ======================================================
// BLUETOOTH HC-05
// ======================================================

SoftwareSerial SerialBT(2, 3);

// ======================================================
// CONFIGURAÇÕES DOS MOTORES
// ======================================================

const int passosPorVolta = 2048;

// ======================================================
// MOTOR 1
// MOTOR DA CATAPULTA
// ======================================================

Stepper motor1(passosPorVolta, 8, 10, 9, 11);

// ======================================================
// MOTOR 2
// MOTOR DE ARMAR/DESARMAR
// ======================================================

Stepper motor2(passosPorVolta, 4, 6, 5, 7);

// ======================================================
// CONFIGURAÇÕES FÍSICAS
// ======================================================

const float passosPorGrau = 2048.0 / 360.0;

float anguloAtual = 0;

const float g = 9.81;
const float m = 0.020;
const float k = 25.0;
const float r = 0.08;

String entrada = "";

// ======================================================
// BOTÃO
// ======================================================

const int botaoPin = 12;

bool sistemaLigado = false;
bool ultimoEstadoBotao = HIGH;

unsigned long ultimoDebounce = 0;
const unsigned long debounceDelay = 50;

// ======================================================
// LED
// ======================================================

const int ledPin = 13;

// ======================================================
// ESTADO DA CATAPULTA
// ======================================================

bool catapultaArmada = false;

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(9600);

  SerialBT.begin(9600);

  motor1.setSpeed(12);
  motor2.setSpeed(12);

  pinMode(botaoPin, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);

  Serial.println("Sistema iniciado.");
  Serial.println("Pressione o botao para ligar/desligar.");

  SerialBT.println("Sistema iniciado.");
}

// ======================================================
// LOOP PRINCIPAL
// ======================================================

void loop() {

  verificarBotao();

  if (!sistemaLigado) {
    return;
  }

  while (SerialBT.available()) {

    char c = SerialBT.read();

    // ENTER
    if (c == '\n' || c == '\r') {

      entrada.trim();

      if (entrada.length() > 0) {

        Serial.print("Recebido: ");
        Serial.println(entrada);

        // ==================================================
        // ARMAR
        // ==================================================

        if (entrada.equalsIgnoreCase("ARMAR")) {

          if (!catapultaArmada) {

            Serial.println("Armando catapulta...");
            SerialBT.println("Armando catapulta...");

            moverMotor2(360);

            catapultaArmada = true;

            Serial.println("Catapulta ARMADA");
            SerialBT.println("Catapulta ARMADA");

          } else {

            SerialBT.println("A catapulta ja esta armada.");
          }
        }

        // ==================================================
        // DESARMAR
        // ==================================================

        else if (entrada.equalsIgnoreCase("DESARMAR")) {

          if (catapultaArmada) {

            Serial.println("Desarmando catapulta...");
            SerialBT.println("Desarmando catapulta...");

            moverMotor2(-360);

            catapultaArmada = false;

            Serial.println("Catapulta DESARMADA");
            SerialBT.println("Catapulta DESARMADA");

          } else {

            SerialBT.println("A catapulta ja esta desarmada.");
          }
        }

        // ==================================================
        // DISTÂNCIA
        // ==================================================

        else {

          float distancia = entrada.toFloat();

          // verifica se realmente digitou número
          if (distancia == 0 && entrada != "0") {

            SerialBT.println("Comando invalido.");
          }

          else if (distancia >= 0.5 && distancia <= 4.0) {

            // ==============================================
            // DEFINE A DISTÂNCIA PRIMEIRO
            // ==============================================

            prepararLancamento(distancia);

            // ==============================================
            // DEPOIS PEDE PARA ARMAR
            // ==============================================

            SerialBT.println("--------------------------------");
            SerialBT.println("Digite ARMAR para armar");
            SerialBT.println("ou DESARMAR para desarmar");
            SerialBT.println("a catapulta.");
            SerialBT.println("--------------------------------");

          } else {

            SerialBT.println("Distancia invalida.");
            SerialBT.println("Digite um valor entre 0.5 e 4.0 metros.");
          }
        }

        entrada = "";
      }
    }

    else {

      entrada += c;
    }
  }
}

// ======================================================
// PREPARA LANÇAMENTO
// ======================================================

void prepararLancamento(float distancia) {

  Serial.println("----------------------");

  Serial.print("Distancia desejada: ");
  Serial.println(distancia);

  // ==================================================
  // EQUAÇÃO
  // ==================================================

  float phi = sqrt(
                 (distancia * m * g)
                 /
                 (r * r * k)
               );

  // radianos -> graus
  float anguloDesejado = phi * 180.0 / PI;

  // ==================================================
  // LIMITES
  // ==================================================

  if (anguloDesejado > 120)
    anguloDesejado = 120;

  if (anguloDesejado < 5)
    anguloDesejado = 5;

  Serial.print("Angulo calculado: ");
  Serial.println(anguloDesejado);

  SerialBT.print("Angulo calculado: ");
  SerialBT.println(anguloDesejado);

  moverMotor1ParaAngulo(anguloDesejado);

  SerialBT.println("Catapulta posicionada!");
}

// ======================================================
// MOVE MOTOR 1
// ======================================================

void moverMotor1ParaAngulo(float novoAngulo) {

  float diferenca = novoAngulo - anguloAtual;

  int passos = diferenca * passosPorGrau;

  Serial.print("Passos motor1: ");
  Serial.println(passos);

  motor1.step(passos);

  anguloAtual = novoAngulo;

  pararMotor1();
}

// ======================================================
// MOVE MOTOR 2
// ======================================================

void moverMotor2(float graus) {

  int passos = graus * passosPorGrau;

  Serial.print("Passos motor2: ");
  Serial.println(passos);

  motor2.step(passos);

  pararMotor2();
}

// ======================================================
// DESLIGA MOTOR 1
// ======================================================

void pararMotor1() {

  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
}

// ======================================================
// DESLIGA MOTOR 2
// ======================================================

void pararMotor2() {

  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
}

// ======================================================
// DESLIGA TODOS
// ======================================================

void pararMotores() {

  pararMotor1();
  pararMotor2();
}

// ======================================================
// BOTÃO LIGA/DESLIGA
// ======================================================

void verificarBotao() {

  bool leitura = digitalRead(botaoPin);

  if (leitura != ultimoEstadoBotao) {

    ultimoDebounce = millis();
  }

  if ((millis() - ultimoDebounce) > debounceDelay) {

    static bool estadoAnterior = HIGH;

    if (leitura == LOW && estadoAnterior == HIGH) {

      sistemaLigado = !sistemaLigado;

      // ==================================================
      // SISTEMA LIGADO
      // ==================================================

      if (sistemaLigado) {

        digitalWrite(ledPin, HIGH);

        entrada = "";

        while (SerialBT.available()) {
          SerialBT.read();
        }

        Serial.println("Sistema LIGADO");
        SerialBT.println("Sistema LIGADO");

        SerialBT.println("--------------------------------");
        SerialBT.println("Digite a distancia desejada");
        SerialBT.println("entre 0.5 m e 4.0 m");
        SerialBT.println("--------------------------------");
      }

      // ==================================================
      // SISTEMA DESLIGADO
      // ==================================================

      else {

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
