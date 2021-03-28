#include "PetFeeder.h"

//Do not define DEBUG if you're not debugging (duh)
//#define DEBUG True

///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::PetFeeder
///////////////////////////////////////////////////////////////////////////////
PetFeeder::PetFeeder() : 
feedingFrequency(0),
feedingHour1(0),
feedingMinute1(0),
feedingHour2(0),
feedingMinute2(0),
feedingHour3(0),
feedingMinute3(0),
feedingAmount(0),
configNumber(0),
configNumberOld(-1),
tagID(-1),
feedingOverride(0),
takePhoto(0),
systemTime(0),
servoPos(0),
fedOnce(0),
fedTwice(0),
fedThrice(0),
currentDay(0),
petVerified(0),
foodReservoirLevel(FOOD_RESEVOIR_MAX_WEIGHT)
{
  pinMode(FOOD_MOTOR_PIN, OUTPUT);
  pinMode(WATER_MOTOR_PIN, OUTPUT);
  pinMode (WATER_LEVEL_PIN, INPUT);
  
  foodScale = new HX711(FOOD_DOUT, FOOD_CLK);
  waterScale = new HX711(WATER_DOUT, WATER_CLK);

  if(foodScale == 0 || waterScale == 0)
  {
#ifdef DEBUG
Serial.println("Invalid HX711 pointer");
#endif
  }

  DistanceSensor = new NewPing(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

  if(DistanceSensor == 0)
  {
#ifdef DEBUG
Serial.println("Invalid NewPing pointer");
#endif
  }

  //-106600 worked for my 40Kg max scale setup 
  float calibration_factor = -96650;

  //Calibration Factor obtained from first sketch
  foodScale->set_scale(-96650);
  //Reset the scale to 0 
  foodScale->tare(); 

  //Calibration Factor obtained from first sketch
  waterScale->set_scale(-96650);
  //Reset the scale to 0 
  waterScale->tare(); 

  sd = new SdFat();
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (sd->begin(chipSelect, SD_SCK_MHZ(50)) == 0) 
  {
#ifdef DEBUG
Serial.println("sd->begin(chipSelect, SD_SCK_MHZ(50)) Failed");
#endif
  }

  rfid = new rdm630(5, 0);
  rfid->begin();

  servo = new Servo();
  servo->attach(6);
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::~PetFeeder
///////////////////////////////////////////////////////////////////////////////
PetFeeder::~PetFeeder()
{
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::parseConfig
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::parseConfig(void)
{
  SdFile configFile("config.txt", O_READ);
  if (configFile.isOpen() == 0) 
  {
#ifdef DEBUG
Serial.println("Could not open config.txt - isOpen() Failed");
#endif
    return -1;
  }

  
  char configFileChars[CONFIG_FILE_SIZE];
  configFile.fgets (configFileChars , CONFIG_FILE_SIZE);
  configFile.close();
  String configFileString(configFileChars);
  
#ifdef DEBUG  
Serial.println("Printing contents of config.txt...");
Serial.println(configFileString);
#endif

  int index = 0;
  String tempStr;
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("N")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find configNumberStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //Next parameter is also +2 because we're extracting one char
    tempStr = configFileString.substring(index+2, index+2+8);
    configNumber = tempStr.toInt(); 
   if(configNumber == configNumberOld)
     return 0;
   configNumberOld = configNumber;
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("F")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingFrequencyStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //Next parameter is also +2 because we're extracting one char
    tempStr = configFileString.substring(index+2, index+3);
    feedingFrequency = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("X")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingHour1Str in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the time format is military time (2 chars)
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingHour1 = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("x")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingMinute1Str in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the time format is military time (2 chars)
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingMinute1 = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("Y")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingHour2Str in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the time format is military time (2 chars)
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingHour2 = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("y")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingMinute2Str in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the time format is military time (2 chars)
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingMinute2 = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("Z")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingHour3Str in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the time format is military time (2 chars)
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingHour3 = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("z")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingMinute3Str in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the time format is military time (2 chars)
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingMinute3 = tempStr.toInt();
  }
  if((index = configFileString.indexOf("U")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find systemTimeStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //Next parameter is also +10 because we're extracting one char
    tempStr = configFileString.substring(index+2, index+2+10);
    systemTime = (unsigned long)tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("A")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingAmountStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the feeding amount is at most a 2 digit number
    tempStr = configFileString.substring(index+2, index+2+2);
    feedingAmount = tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("R")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find tagIDStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //+2 is because the rfid is at most a 7 digit number
    tempStr = configFileString.substring(index+2, index+2+7);
    tagID = (unsigned long)tempStr.toInt();
  }
  //Set index equal to the position where the field name is
  //If indexOf returns -1, the specified field name was not found
  if((index = configFileString.indexOf("O")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find feedingOverrideStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //Next parameter is also +2 because we're extracting one char
    tempStr = configFileString.substring(index+2, index+3);
    feedingOverride = (bool)tempStr.toInt();
  }
  if((index = configFileString.indexOf("P")) == -1)
  {
#ifdef DEBUG
Serial.println("Could not find takePhotoStr in configFileString");
#endif
  return -1;
  }
  else
  {
    //+2 is to skip the current char and the space in the config string
    //Next parameter is also +2 because we're extracting one char
    tempStr = configFileString.substring(index+2, index+3);
    takePhoto = (bool)tempStr.toInt();
  }

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::dispenseFoodScheduled
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::dispenseFoodScheduled(void)
{
  if(((feedingHour1 == hour()) && (feedingMinute1 == minute()) && fedOnce ==   false ||
      (feedingHour2 == hour()) && (feedingMinute2 == minute()) && fedTwice ==  false ||
      (feedingHour3 == hour()) && (feedingMinute3 == minute()) && fedThrice == false   ) == 0)
    return 0;

  if(feedingHour1 == hour() && feedingMinute1 == minute())
    fedOnce = true;
  else if(feedingHour2 == hour() && feedingMinute2 == minute())
    fedTwice = true;
  else if(feedingHour3 == hour() && feedingMinute3 == minute())
    fedThrice = true;

  dispenseFood();

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::weighAndStoreFood
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::weighAndStoreFood()
{
////   double data[10];
////   do
////   {
////     int i;
////     for (i=0; i<10; ++i)
////     {
////       data[i] = foodScale->get_units();
////     }
////   }
////   while(calculateSD(data) > LARGEST_SD_VALUE && 0);
//
////  double averageWeight;
//  // averageWeight = calculateMean(data);
//
//  File patternsFile = sd->open("pat.txt", FILE_WRITE);
//  if (patternsFile == 0) 
//  {
////#ifdef DEBUG
//Serial.print("Could not open ");
//Serial.print("pat.txt");
//Serial.println(" - isOpen() Failed");
////#endif
//    return -1;
//  }
//// Serial.println(foodScale->get_units());
//  patternsFile.print("Animal ate or drank water at");
////  patternsFile.print(foodScale->get_units(), 3); 
////                                                   patternsFile.print("or drank water at ");
//   patternsFile.print(" of food at ");
////  patternsFile.print(hour());
//  patternsFile.print(":");
////  patternsFile.print(minute());
//  patternsFile.print(" ");
////  patternsFile.print(month());
//  patternsFile.print("/");
////  patternsFile.print(day());
//  patternsFile.print("/");
////  patternsFile.print(year());
//  patternsFile.println("]");
//  patternsFile.println("segszgvgzsvsdzvfzvzfvbz");
//  patternsFile.close();
  
  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::getWaterLevel
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::getWaterLevel(void)
{
  if(analogRead(WATER_LEVEL_PIN) < 512)
    return waterLevel = 0;
  else if(analogRead(WATER_LEVEL_PIN) > 512)
    return waterLevel = 1;
  else
    return waterLevel = -1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::verifyPet
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::verifyPet(void)
{
  if(closeEnough() == 0)
    return 0;
    
  if(rfid->available() == false)
  	return 0;

  byte data[6];
  byte length;
  rfid->getData(data,length);
  unsigned long result = 
    ((unsigned long int)data[1]<<24) + 
    ((unsigned long int)data[2]<<16) + 
    ((unsigned long int)data[3]<<8)  + 
    data[4];

  if(result == tagID)
  {
    delete rfid;
    rfid = new rdm630(5, 0);
    openDoor();
    // weighAndStoreFood();
    rfid->begin();
    return 1;
  }
  else 
  	return -1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::setFeederTime
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::setFeederTime(void)
{
  setTime(systemTime);
  currentDay = day();

  return 1;
}

///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::updateSchedule
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::updateSchedule(void)
{
  if(currentDay != day())
  {
    fedOnce   = false;
    fedTwice  = false;
    fedThrice = false;
  }

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::dispenseWater
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::dispenseWater(void)
{
  //dispenseTime = (WATER_PORTION)mL * min/100mL * 60sec/min * 1000ms/sec
  int dispenseTime = (int)WATER_PORTION*MIN_PER_ML*60*1000;

  while(BOWL_VOLUME - 1000 > 1000*waterScale->get_units())
  {
    digitalWrite(WATER_MOTOR_PIN, HIGH);
    delay(dispenseTime);
    digitalWrite(WATER_MOTOR_PIN, LOW);
  }

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::calculateSD
///////////////////////////////////////////////////////////////////////////////
double PetFeeder::calculateSD(double data[])
{
  double sum = 0.0;
  double mean = 0.0;
  double standardDeviation = 0.0;

  int i;
  for(i = 0; i < 10; ++i)
  {
    sum += data[i];
// Serial.println("In calculateSD: ");
// Serial.println(data[i]);
  }

  mean = sum/10;

  for(i = 0; i < 10; ++i)
  {
    standardDeviation += pow(data[i] - mean, 2);
  }

  return sqrt(standardDeviation / 10);
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::calculateMean
///////////////////////////////////////////////////////////////////////////////
double PetFeeder::calculateMean(double data[])
{
  double sum = 0.0;
  double mean = 0.0;

  int i;
  for(i = 0; i < 10; ++i)
  {
    sum += data[i];
// Serial.println("In calculateMean: ");
// Serial.println(data[i]);
  }

  mean = sum/10; 

  return mean;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::closeEnough
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::closeEnough()
{
  objectDistance = DistanceSensor->ping_cm();

  if(objectDistance < MIN_DIST_TO_SCAN && objectDistance > 0)
    return 1;
  else
  	return 0;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::openDoor
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::openDoor()
{    
  for(servoPos; servoPos <= 180; servoPos++) 
  {
    servo->write(servoPos);
    delay(15);
  }

  while(closeEnough() == 1)
  {
    delay(250);
  }

  for(servoPos; servoPos >= 0; servoPos--) 
  {
    servo->write(servoPos);
    delay(15);
  }

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::updateFoodReservoir
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::updateFoodReservoir()
{
  if(analogRead(FOOD_RESEVOIR_PIN) < 512)
  {
    foodReservoirLevel = FOOD_RESEVOIR_MAX_WEIGHT;
    return 0;
  }
  else if(analogRead(FOOD_RESEVOIR_PIN) > 512)
  {
    return  1;
  }
  else
    return -1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::monitorSerial
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::monitorSerial()
{
  char incommingSignal = Serial.read();

  //Dispense food action
  switch(incommingSignal)
  {
    case 'o':
      if(dispenseFood() == 1)
        Serial.print("o");
      else
#ifdef DEBUG
Serial.print("dispenseFood() failed");
#endif
      break;

//     case 'p':
//       feedingPatternsFile = sd->open("pat.txt", O_RDWR);
//       if (feedingPatternsFile.isOpen() == 0) 
//       {
// #ifdef DEBUG
// Serial.print("Could not open ");
// Serial.print("patterns.txt");
// Serial.println(" - isOpen() Failed");
// #endif
//         return -1;
//       }

//       while(feedingPatternsFile.available())
//       {
//         Serial.write(feedingPatternsFile.read());
//       }      
//       Serial.print("p");
//       feedingPatternsFile.truncate(0);
//       feedingPatternsFile.close();
//       break;

    case 'f':
      Serial.print(foodReservoirLevel);
      // Serial.print("f");
      break;

    case 'w':
      if(getWaterLevel() == 1)
        Serial.print("High");
      else 
      	Serial.print("Low");
      break;

    case 'c':
      memset(newConfig, 0, NEW_CONFIG_SIZE);
      for(int i=0, incommingSignal=Serial.read(); incommingSignal != 'c' && i < 100; i++, incommingSignal=Serial.read())
        newConfig[i] = incommingSignal;
      break;
    case 'x':
      parseNewConfig();
      break;
  }

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::dispenseFood
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::dispenseFood()
{
  if(feedingAmount <= 0)
  {
#ifdef DEBUG
Serial.println("Invalid food quantity");
#endif
    return -1;  
  }

  double ozPerSec = REV_PER_SEC*OZ_PER_REV;
  double secPerOz = 1/ozPerSec;
  int dispenseTime = (int)(feedingAmount*secPerOz*1000);

  digitalWrite(FOOD_MOTOR_PIN, HIGH);
  delay(dispenseTime);
  digitalWrite(FOOD_MOTOR_PIN, LOW);

  foodReservoirLevel = foodReservoirLevel - feedingAmount;

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::monitorFoodReset
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::monitorFoodReset()
{
  if(analogRead(FOOD_RESEVOIR_PIN) > 512)
    foodReservoirLevel = FOOD_RESEVOIR_MAX_WEIGHT;
  else if(analogRead(FOOD_RESEVOIR_PIN) < 512)
    return 0;

  return 1;
}


///////////////////////////////////////////////////////////////////////////////
/// PetFeeder::parseNewConfig
///////////////////////////////////////////////////////////////////////////////
int PetFeeder::parseNewConfig()
{
  String newConfigFileString(newConfig);
  Serial.println(newConfigFileString);

  return 1;
}
