/* Tino core 1.3.5 */
/* by Cédric Couvrat et plein d'autres gens */
/* capteur rfid + telemetre ultrason + capteur de son + com serie  */

#include <SPI.h>
#include <RFID.h>

RFID monModuleRFID(10,9); //définition des broches (digit) pour le lecteur rfid
int UID[5]; //définition du tableau de valeurs transmises par rfid
const int capteurson = 4 ;// définition de la broche out (digit) du capteur de son
const int lcard = 8;// broche connecté à la LED témoin RFID + resistance

/* Définition du capteur Ultrason HC-SR04 */
int trig = 3;
int echo = 2;
long lecture_echo;
long cm;
// timer
float sonartp1 = millis();
int delaysonar = 500; // 1seconde = 1000 ms

/* Communication serie vers arduino*/
boolean DEBUG=false; // Active l'affichage des messages de debug
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int cptin=0; // comptage du nb de caractères reçus

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  monModuleRFID.init();  
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(lcard, OUTPUT);
  digitalWrite(lcard, LOW);
  pinMode(capteurson, INPUT);
  pinMode(echo, INPUT);
  // showInstructions();
}

void loop()
{

/* Utilisation du capteur de son */
if(digitalRead(capteurson)== HIGH)
   {
   //digitalWrite(LED, HIGH);// Allumer la LED
   Serial.println("bruit");
   delay(10);// Temps de traitement
   //digitalWrite(LED, LOW);// Eteindre la LED   
}

/* Utilisation du capteur Ultrason HC-SR04 */
  if (millis()-sonartp1 > delaysonar) {
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    lecture_echo = pulseIn(echo, HIGH);
    cm = lecture_echo / 58;
    Serial.print("d:");
    Serial.println(cm);
    sonartp1=millis();
  }

/* Utilisation du module RFID */
  if (monModuleRFID.isCard()) {  
     if (monModuleRFID.readCardSerial()) {
            Serial.print("c:");
            for(int i=0;i<=4;i++)
            {
              UID[i]=monModuleRFID.serNum[i];
              Serial.print(UID[i],HEX);
            }
            Serial.println();
            digitalWrite(lcard, HIGH);
            delay(100);
            digitalWrite(lcard, LOW);
          }          
          monModuleRFID.halt();
  }

/* Communication serie vers arduino */
  // print the string when a newline arrives:
  if (stringComplete) {
    debugMsg(inputString);

    // chaineReception.trim(); // enlève les espaces
    analyseChaine(inputString); // appelle la fonction d'analyse de la chaine en réception
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
    
} // fin loop


/*
 Com serie :  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();

    // add it to the inputString:
    inputString += inChar;
    cptin++;
   
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' || cptin==10) {
      stringComplete = true;
      cptin=0;
      debugMsg(inputString);
    }
  }
}

// --------------------------------------------------
//  Analyse les instructions recus via le serie
// --------------------------------------------------
void analyseChaine(String chaineRecue) { // fonction d'analyse de la chaine recue

      int valeur=0; // variable utile
      String chaineTest; // objet STring pour analyse chaine recue

      // convertie les lettres en majuscules, pour rendre les commandes case insensitives...
      chaineRecue.toUpperCase();
      if (chaineRecue.startsWith("TINO"))  tinoVersion();
      if (chaineRecue.startsWith("HELP"))  showInstructions();
      if (chaineRecue.startsWith("LED")) doLed();
      if (chaineRecue.startsWith("DEBUGON")) DEBUG=true;
      if (chaineRecue.startsWith("DEBUGOFF")) DEBUG=false;
}
// Affiche les messages en retour sur le port série
void debugMsg(String todisplay) {
  if (DEBUG==true)  Serial.println(todisplay);
}

// -----------------------------------------------------------
//  Affiche toutes les commandes quand HELP est envoyé
// -----------------------------------------------------------
void showInstructions() {
  Serial.println(F("LED : allume la led "));
  Serial.println(F("DEBUGON : envoie en serie les messages de gestion des traitements et erreurs")); 
  Serial.println(F("DEBUGOFF : n'envoie aucun message d'erreur, comportement par defaut"));   
}

void tinoVersion() {
  Serial.println(F("Tino project - Tino Core"));
  Serial.println(F("Version : 1.3.5"));
  Serial.println(F("Derniere mise a jour le : 04.01.2017")); 
  Serial.println(F("Parametrage vitesse port serie : 96000 Bauds"));
}

void doLed() {
  digitalWrite(lcard, HIGH);// Allumer la LED
}

