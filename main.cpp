#include "mbed.h"
#include <cstdio>

// Definieer pinnen voor motorcontroller
DigitalOut enable(D9);  // Enable pin (ENA)
DigitalOut in1(D10);     // Input 1 (IN1)
DigitalOut in2(D11);     // Input 2 (IN2)

// Definieer pinnen voor IR sensor
DigitalIn irSensor(D2); // Aansluiting van de IR-sensor, gele draad digitaal ipv analoog

// Definieer pinnen voor Stop en Start knoppen 
DigitalIn Stopknop(D8); // Aansluiting van de Stopknop
DigitalIn Startknop(D5); // Aansluiting van de Startknop

// Definieer pinnen voor Ultrasoon/objecten sensor
DigitalOut trigger(D6);
DigitalIn echo(D7);

// variabelen voor de switch case voor het rijden
enum class Uitvoering { Vooruit, Ontwijk };
bool entry = true;

int main() {
  Uitvoering now = Uitvoering::Vooruit;
  Timer timer; // Timer variabele verplaatst naar bredere scope

  while (true) {
    switch (now) {
      case Uitvoering::Vooruit:
        if (Startknop == true) {
          enable = 1;
          in1.write(1);
          in2.write(0);

          // Genereer een triggerpuls van 10 microseconden om een meting te starten
          trigger = 1;
          wait_us(10);
          trigger = 0;

          // Wacht tot het echo-signaal hoog wordt
          while (echo == 0) {}

          // Wacht tot het echo-signaal laag wordt
          timer.start();
          while (echo == 1) {}

          // Stop de timer en bepaal of er een object binnen bereik is (1) of niet (0)
          timer.stop();
          float pulseWidth = timer.elapsed_time().count();
          int presence = (pulseWidth < 2000) ? 1 : 0; // Met deze line de objecten detecteer afstand vastleggen

          if (irSensor.read() == 0 || presence == 1) { // Waarnemen van hoogte verschil of object = omschakelen naar Ontwijk case
            now = Uitvoering::Ontwijk;
          }
        }
        break;

      case Uitvoering::Ontwijk:
        if (entry) { // Achteruit rijden
          in1.write(0);
          in2.write(1);
          timer.start();
        }

        if (std::chrono::duration<float>{timer.elapsed_time()}.count() > 0.5) {
          // Servomotor laten draaien
          ThisThread::sleep_for(1s);
          now = Uitvoering::Vooruit;
          entry = true;
        }
        break;
    }

    // Print de status van de digitale pinnen elke 5 seconden
    if (std::chrono::duration<float>{timer.elapsed_time()}.count() >= 2.0) {
      printf("Status digitale pinnen:\n");
      printf("Startknop: %d\n", Startknop.read());
      printf("Stopknop: %d\n", Stopknop.read());
      printf("irSensor: %d\n", irSensor.read());
      printf("\n");

      timer.reset();
    }
  }
}
