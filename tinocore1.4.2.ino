/*-----------------------------------------
 * Tino core 1.4.2
 * by Cédric Couvrat et plein d'autres gens
 * ----------------------------------------
 *  branche 1.4 données typées
 *  capteur rfid + telemetre ultrason + capteur de son + 2 servo + matrice led + com serie
*/

#include <SPI.h>
#include <RFID.h>
#include <Servo.h>

RFID monModuleRFID(10,9); //définition des broches (digit) pour le lecteur rfid
int UID[5]; //définition du tableau de valeurs transmises par rfid
const int capteurson = 4 ;// définition de la broche out (digit) du capteur de son
const int lcard = 8;// broche connecté à la LED témoin RFID + resistance

/* Définition des servo-moteurs */
#define PIN_SERVO_1 A0
#define PIN_SERVO_2 A1
Servo myservo1;
Servo myservo2;
int pos = 0;    // variable de position du servo 

/* Définition de la matrice de LEDS 8x8 Max7219 */
unsigned char j;
// Déclaration des broches pour la matrice
int Max7219_pinCLK = 7; 
int Max7219_pinCS = 6;
int Max7219_pinDIN = 5;
// Motifs prédéfinis
unsigned char disp1[7][8]={
{0x0,0x0,0x81,0x81,0x42,0x3C,0x0,0x0}, //sourir -> 0
{0x0,0x0,0x0,0x3C,0x42,0x81,0x81,0x0}, //triste -> 1
{0x0,0x0,0x0,0x0,0xFF,0x0,0x0,0x0}, //neutre -> 2
{0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C}, // bouche bée -> 3
{0x0,0x0,0x8,0x55,0xA2,0x0,0x0,0x0}, // bouche tremblante -> 4
{0x0,0x81,0x81,0x42,0x3C,0x0,0x0,0x0}, //sourir2 -> 5
{0x18,0x24,0x42,0x24,0x8,0x8,0x0,0x8}, //interro -> 6

};

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
  pinMode(PIN_SERVO_1, OUTPUT);
  pinMode(PIN_SERVO_2, OUTPUT);
  myservo1.attach(PIN_SERVO_1);
  myservo2.attach(PIN_SERVO_2);
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  delay(50);  //Initialiser
  // showInstructions();
  Init_MAX7219();
  m_smile();
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
//  Analyse les instructions recues via le serie
// --------------------------------------------------
void analyseChaine(String chaineRecue) { // fonction d'analyse de la chaine recue

      int valeur=0; // variable utile
      String chaineTest; // objet STring pour analyse chaine recue

      // convertie les lettres en majuscules, pour rendre les commandes case insensitives...
      chaineRecue.toUpperCase();
      if (chaineRecue.startsWith("TINO"))  tinoVersion();
      if (chaineRecue.startsWith("HELP"))  showInstructions();
      if (chaineRecue.startsWith("LED")) doLed();
      if (chaineRecue.startsWith("SOURIR")) {
        m_smile();
        myservo1.write(80);
        myservo2.write(100);
        }
      if (chaineRecue.startsWith("SOURIR2")) {
        m_smile2();
        myservo1.write(90);
        myservo2.write(90);
        }
      if (chaineRecue.startsWith("NEUTRE")) {
        m_neutral();
        myservo1.write(90);
        myservo2.write(90);
        }
      if (chaineRecue.startsWith("BEE")) {
        m_gap();
        myservo1.write(100);
        myservo2.write(80);
        }
      if (chaineRecue.startsWith("PEUR")) {
        m_tremb();
        myservo1.write(70);
        myservo2.write(110);
        }
      if (chaineRecue.startsWith("TRISTE")) {
        m_sad();
        myservo1.write(100);
        myservo2.write(80);
        }
      if (chaineRecue.startsWith("VU")) {
        m_interro();
        myservo1.write(90);
        myservo2.write(90);
        }
      if (chaineRecue.startsWith("DEBUGON")) DEBUG=true;
      if (chaineRecue.startsWith("DEBUGOFF")) DEBUG=false;
}
// Affiche les messages en retour sur le port série
void debugMsg(String todisplay) {
  if (DEBUG==true)  Serial.println(todisplay);
}

// -----------------------------------------------------------
//  Affichage série
// -----------------------------------------------------------
//  Affiche toutes les commandes quand HELP est envoyé
void showInstructions() {
  Serial.println(F("LED : allume la led "));
  Serial.println(F("DEBUGON : envoie en serie les messages de gestion des traitements et erreurs")); 
  Serial.println(F("DEBUGOFF : n'envoie aucun message d'erreur, comportement par defaut"));   
}
//  Affiche la version de Tino quand TINO est envoyé
void tinoVersion() {
  Serial.println(F("Tino project - Tino Core"));
  Serial.println(F("Version : 1.4.2"));
  Serial.println(F("Derniere mise a jour le : 04.01.2017")); 
  Serial.println(F("Parametrage vitesse port serie : 96000 Bauds"));
}

// ------------------------------------------------------------
// Matrice LEDs
// ------------------------------------------------------------
void m_smile()
{
   for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[0][i-1]);
}
void m_smile2()
{
   for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[5][i-1]);
}

void m_sad()
{
   for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[1][i-1]);
}

void m_neutral()
{
   for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[2][i-1]);
}

void m_gap()
{
   for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[3][i-1]);
}

void m_tremb()
{
   for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[4][i-1]);
}
void m_interro()
{
  for(int i=1;i<9;i++)  //ecriture des 8 lignes de la matrice de leds pour afficher le motif
    Write_Max7219(i,disp1[6][i-1]);
}
//Ecriture d'un caractere 8x8
void Write_Max7219_byte(unsigned char DATA)
{   
       unsigned char i;
       digitalWrite(Max7219_pinCS,LOW);      
       for(i=8;i>=1;i--)
          {       
             digitalWrite(Max7219_pinCLK,LOW);
             digitalWrite(Max7219_pinDIN,DATA&0x80);// Extracting a bit data
             DATA = DATA<<1;
             digitalWrite(Max7219_pinCLK,HIGH);
           }                                 
}
// Ecriture elementaire d une seule rangee
void Write_Max7219(unsigned char address,unsigned char dat)
{
        digitalWrite(Max7219_pinCS,LOW);
        Write_Max7219_byte(address);           //address，code of LED
        Write_Max7219_byte(dat);               //data，figure on LED
        digitalWrite(Max7219_pinCS,HIGH);
}
// Initialisation du module Max 7219
void Init_MAX7219(void)
{
 Write_Max7219(0x09, 0x00);       //decoding ：BCD
 Write_Max7219(0x0a, 0x03);       //brightness
 Write_Max7219(0x0b, 0x07);       //scanlimit；8 LEDs
 Write_Max7219(0x0c, 0x01);       //power-down mode：0，normal mode：1
 Write_Max7219(0x0f, 0x00);       //test display：1；EOT，display：0
}

// ------------------------------------------------------------
// Positions servos
// ------------------------------------------------------------
void s_smile() {
    myservo1.write(80);
    myservo2.write(100);
}
void s_sad() {
    myservo1.write(110);
    myservo2.write(60);
}
void s_neutral() {
    myservo1.write(90);
    myservo2.write(90);
}

// ------------------------------------------------------------
// Autres fonctions
// ------------------------------------------------------------
void doLed() {
  digitalWrite(lcard, HIGH);// Allumer la LED
}

