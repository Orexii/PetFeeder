#include "PetFeeder.h"

PetFeeder *petFeeder = 0;

void setup() 
{ 
  Serial.begin(9600);
  delay(5000);
  Serial.println("Starting setup");
  // put your setup code here, to run once:
  petFeeder = new PetFeeder();

  petFeeder->parseConfig();
  petFeeder->setFeederTime();
}

void loop() 
{
  // put your main code here, to run repeatedly:

  petFeeder->dispenseFoodScheduled();
  
  petFeeder->verifyPet();

  petFeeder->updateSchedule();

  //petFeeder->dispenseWater();

  petFeeder->updateFoodReservoir();

  petFeeder->monitorSerial();

  petFeeder->monitorFoodReset();
}

