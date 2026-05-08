#include <SoftwareSerial.h>
#include <Stepper.h>
#include <math.h>

// ======================================================
// BLUETOOTH
// ======================================================

SoftwareSerial bluetooth(2, 3);

// ======================================================
// CONFIGURAÇÕES DOS MOTORES
// ======================================================

const int passosPorVolta = 2048;

// ======================================================
// MOTOR 1
// ======================================================
// IN1 IN3 IN2 IN4

Stepper motor1(passosPorVolta, 8, 10, 9, 11);

// ======================================================
// MOTOR 2
// ======================================================
// IN1 IN3 IN2 IN4

Stepper motor2(passosPorVolta, 4, 6, 5, 7);

// ======================================================
// CONFIGURAÇÕES GERAIS
// ======================================================

const float passosPorGrau = 2048.0 / 360.0;

float anguloAtual = 0;

// ======================================================
// PARÂMETROS FÍSICOS
// ======================================================

const float g = 9.81;

// massa do projétil
const float m = 0.020;

// constante elástica
const float k = 25.0;

// raio do braço
const float r = 0.08;

// ======================================================

String entrada = "";

// ======================================================

void setup() {

  bluetooth.begin(9600);
  Serial.begin(9600);

  motor1.setSpeed(12);
  motor2.setSpeed(12);

  bluetooth.println("Digite distancia entre 0.5 e 4 metros");

  Serial.println("Sistema iniciado");
}

// ======================================================

void loop() {

  while (bluetooth.available()) {

    char c = bluetooth.read();

    // ignora espaços
    if (c == ' ')
      continue;

    // ENTER
    if (c == '\n' || c == '\r') {

      if (entrada.length() > 0) {

        float distancia = entrada.toFloat();

        Serial.print("Recebido: ");
        Serial.println(distancia);

        // limpa imediatamente
        entrada = "";

        if (distancia >= 0.5 && distancia <= 4.0) {

          prepararLancamento(distancia);
        }
        else {

          bluetooth.println("Distancia invalida");
          Serial.println("Distancia invalida");
        }
      }
    }

    // aceita apenas números e ponto
    else if (
      (c >= '0' && c <= '9')
      || c == '.'
    ) {

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
  // EQUAÇÃO:
  //
  // phi = sqrt((R*m*g)/(r²*k))
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

  bluetooth.print("Angulo: ");
  bluetooth.println(anguloDesejado);

  moverParaAngulo(anguloDesejado);

  bluetooth.println("Catapulta armada!");
}

// ======================================================
// MOVE OS DOIS MOTORES
// ======================================================

void moverParaAngulo(float novoAngulo) {

  float diferenca = novoAngulo - anguloAtual;

  int passos = diferenca * passosPorGrau;

  Serial.print("Passos: ");
  Serial.println(passos);

  // ==================================================
  // MOTORES GIRAM JUNTOS
  // ==================================================

  if (passos > 0) {

    for (int i = 0; i < passos; i++) {

      motor1.step(1);
      motor2.step(1);
    }
  }
  else {

    for (int i = 0; i < abs(passos); i++) {

      motor1.step(-1);
      motor2.step(-1);
    }
  }

  anguloAtual = novoAngulo;

  pararMotores();
}

// ======================================================
// DESLIGA BOBINAS
// ======================================================

void pararMotores() {

  // MOTOR 1
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);

  // MOTOR 2
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
}
