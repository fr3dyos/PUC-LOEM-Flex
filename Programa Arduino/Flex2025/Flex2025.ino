/*
  ##Programado por Fredy Osorio em 01/2025 para o LOEM PUC-Rio.
  ing.fredyosorio@gmail.com
  
  ## Disponivel em Github https://github.com/fr3dyos/PUC-LOEM-Flex
  ## Sistema de Controle de Motor de Passo Arduino

  Este sketch Arduino implementa um sistema versátil de controle de motor de passo com as seguintes características:

  - Interface de comando serial para fácil controle do motor
  - Rastreamento de posição com uma variável global
  - Comandos de movimento para frente e para trás
  - Capacidades de loop e loop infinito
  - Comando de parada para interromper movimentos em andamento
  - Zeragem de posição para calibração

  O sistema usa três pinos para controlar o motor de passo:
  - Pino de habilitação (5): Habilita/desabilita o driver do motor
  - Pino de passo (11): Controla passos individuais
  - Pino de direção (12): Define a direção de rotação

  As funções principais incluem:
  - moveMotor(): Move o motor um número específico de passos
  - loopMotor(): Executa uma sequência de loop de movimentos para frente e para trás
  - infMotor(): Executa um loop infinito de movimentos para frente e para trás
  - stopMotor(): Interrompe o movimento do motor
  - printPosition(): Exibe a posição atual do motor
  - zeroPosition(): Redefine o contador de posição para zero

  Os usuários podem controlar o motor através de comandos de texto simples enviados via monitor serial, tornando-o adequado para várias aplicações que requerem controle preciso do motor e rastreamento de posição.
*/

#include <AccelStepper.h>

const int enablePin = 5;
const int stepPin = 11;
const int dirPin = 12;

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup() {
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);  // Disable the driver initially
  
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(200);
  
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    Serial.println(command);
    command.trim();
    executeCommand(command);
  }
  
  stepper.run();
}

void executeCommand(String command) {
  char direction = command.charAt(0);
  int steps = command.substring(1).toInt();

  switch (direction) {
    case 'O':
      moveMotor(steps, false);
      break;
    case 'C':
      moveMotor(steps, true);
      break;
    case 'S':
      stopMotor();
      break;
    case 'L':
      loopMotor(command);
      break;
    case 'I':
      infMotor(command);
      break;
    case 'P':
      printPosition();
      break;
    case 'Z':
      zeroPosition();
      break;
    case 'A':
      setSpeedAndAcceleration(command);
      break;
    default:
      Serial.println("Invalid command");
  }
}

void moveMotor(int steps, bool open) {
  digitalWrite(enablePin, LOW);  // Enable the driver
  stepper.moveTo(open ? stepper.currentPosition() + steps : stepper.currentPosition() - steps);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  digitalWrite(enablePin, HIGH);  // Disable the driver
  printPosition();
}

void stopMotor() {
  Serial.println("Stopping motor");
  stepper.stop();
  digitalWrite(enablePin, HIGH);  // Disable the driver
  delay(100);
  printPosition();
}

void loopMotor(String command) {
  int steps = command.substring(1, 4).toInt();
  int loopCount = command.substring(4).toInt();

  Serial.print("Steps: ");
  Serial.print(steps);
  Serial.print("   Loop: ");
  Serial.println(loopCount);

  for (int i = 1; i <= loopCount; i++) {
    moveMotor(steps, true);   // Open
    delay(500);
    moveMotor(steps, false);  // Close
    delay(500);
    Serial.print("L");
    Serial.println(i);

    if (Serial.available() > 0) {
      if (Serial.read() == 'S') {
        stopMotor();
        Serial.println("STOP");
        return;
      }
    }
  }
  Serial.println("Loop sequence completed");
  printPosition();
}

void infMotor(String command) {
  int steps = command.substring(1, 4).toInt();
  String acc = command.substring(4);
  setSpeedAndAcceleration("A0500"+acc);
  stepper.runSpeed();
  float i = 0;
  while (true) {
    moveMotor(steps, true);
    i += 0.5;
    Serial.print("L");
    Serial.println(i);   
    moveMotor(steps, false);
    i += 0.5;
    Serial.print("L");
    Serial.println(i);

    if (Serial.available() > 0) {
      if (Serial.read() == 'S') {
        stopMotor();
        Serial.println("STOP");
        setSpeedAndAcceleration("A0500200");
        return;
      }
    }
  }
}

void printPosition() {
  Serial.print("P");
  Serial.println(stepper.currentPosition());
}

void zeroPosition() {
  stepper.setCurrentPosition(0);
  printPosition();
}

void setSpeedAndAcceleration(String command) {
  int spd = command.substring(1, 5).toInt();
  int acc = command.substring(5).toInt();
  stepper.setMaxSpeed(spd);
  stepper.setSpeed(spd);
  stepper.setAcceleration(acc);
}
