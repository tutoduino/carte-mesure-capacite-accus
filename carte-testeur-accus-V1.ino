// Mesure de la tension et/ou de la capacité d'un accumulteur
// https://tutoduino.fr/
// Copyleft 2020

// Librairie pour l'afficheur OLED
// https://github.com/greiman/SSD1306Ascii
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <PinChangeInt.h>

// Adresse de l'afficheur OLED sur le bus I2C
#define I2C_ADDRESS 0x3C

// Declaration de l'ecran OLED
SSD1306AsciiAvrI2c oled;

// variables utilisees pour les routines
// d'interruptions pour la gestion des boutons
unsigned long last_interrupted = 0;
volatile bool bouton_haut_appuye = false;
volatile bool bouton_bas_appuye = false;
volatile bool bouton_ok_appuye = false;

// Le bouton Haut est relie a la broche PC1 (15)
#define brocheBoutonHaut 15
// Le bouton Bas est relie a la broche PC2 (16)
#define brocheBoutonBas 16
// Le bouton Ok est relie a la broche PC3 (17)
#define brocheBoutonOk 17

// Le bouton est conçu en mode pull-up, l'appel a la fonction
// digitalRead retourne la valeur LOW lorsque le bouton est appuye
#define BOUTON_APPUYE LOW

// Machine a etats pour la gestion du menu
enum machineEtatType_t: uint8_t {
    ME_MENU_TENSION,
    ME_MENU_CAPACITE,
    ME_MENU_ETALONNAGE,
    ME_MENU_CAPA_NIMH,
    ME_MENU_CAPA_LIION
};
machineEtatType_t machineEtat = ME_MENU_TENSION;


// Constantes etalonnees 
// ---------------------
// Valeurs de courants de decharge en mA
#define COURANT1 151
#define COURANT2 293
#define COURANT3 434
// Valeur de la tension de reference
#define VREF 1.075
// Rapport pont diviseur utilise sur A0
#define PDIV0 4.15

// Seuils bas en tension des accus
// Ne pas descendre sous ces seuils lors de la
// mesure de capacite, sous peine de les endommager
#define SEUIL_BAS_TENSION_ACCU_NIMH     0.80
#define SEUIL_BAS_TENSION_ACCU_LIION    2.00

// Mesure de la tension d'un accumulateur
void mesureTension() {
  float u;

  // La mesure en tension se fait à faible
  // courant de decharge COURANT1
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  
  while (bouton_ok_appuye == false) {
    oled.clear();
    oled.set2X();    
    oled.println("Tension");
    oled.set1X();    
    u = PDIV0*analogRead(A0)*VREF/1023.0;
    oled.println("");
    oled.println("U = " + String(u) + " V");  
    oled.println("");
    oled.println("ok -> terminer");
    delay(2000);
  }
  
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  bouton_ok_appuye = false;
}

void mesureCapacite(float seuil) {
  float u;
  uint16_t q;
  unsigned long initTimeSec, dureeMesureSec;

  initTimeSec = millis()/1000;

  // La mesure de la capacite de l'accu se fait à 
  // fort courant de decharge COURANT3 
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  
  u = PDIV0*analogRead(A0)*VREF/1023.0;
  
  while ((u >= seuil) && (bouton_ok_appuye == false)) {
    oled.clear();
    oled.set2X();
    oled.println("Capacite");
    oled.set1X(); 
    u = PDIV0*analogRead(A0)*VREF/1023.0;
    oled.println("");
    oled.println("U = " + String(u) + " V");  
    dureeMesureSec = millis()/1000-initTimeSec;
    q = COURANT3*dureeMesureSec/3600;
    oled.println("Q = "+String(q)+" mAh");  
    delay(2000);
  }

  oled.println("mesure terminee");
  oled.println("ok -> fin");
  
  bouton_ok_appuye = false;
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  
  while (bouton_ok_appuye == false) {};
  bouton_ok_appuye = false;
}


// Etalonnage du circuit
//
void etalonnage() {
  float U0;

  oled.clear();
  oled.set2X(); 
  oled.println("Etalonnage");
  oled.set1X(); 
  oled.println("");
  oled.println("Broche PD3 : Low ");
  oled.println("Broche PD4 : Low ");
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  delay(500);
  U0 = PDIV0*analogRead(A0)*VREF/1023.0;
  oled.println("U = " + String(U0) + " V");  
  oled.println("");
  oled.println("ok -> suivant");  
  while (bouton_ok_appuye == false) {};
  bouton_ok_appuye = false;
    
  oled.clear();
  oled.set2X(); 
  oled.println("Etalonnage");
  oled.set1X(); 
  oled.println("");
  oled.println("Broche PD3 : Low ");
  oled.println("Broche PD4 : High ");
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  delay(500);  
  U0 = PDIV0*analogRead(A0)*VREF/1023.0;
  oled.println("U = " + String(U0) + " V");  
  oled.println("");
  oled.println("ok -> suivant");  
  while (bouton_ok_appuye == false) {};
  bouton_ok_appuye = false;
  
  oled.clear();
  oled.set2X(); 
  oled.println("Etalonnage");
  oled.set1X(); 
  oled.println("");
  oled.println("Broche PD3 : High ");
  oled.println("Broche PD4 : Low ");
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);
  delay(500);  
  U0 = PDIV0*analogRead(A0)*VREF/1023.0;
  oled.println("U = " + String(U0) + " V");  
  oled.println("");
  oled.println("ok -> suivant");  
  while (bouton_ok_appuye == false) {};
  bouton_ok_appuye = false;
  
  oled.clear();
  oled.set2X(); 
  oled.println("Etalonnage");
  oled.set1X(); 
  oled.println("");
  oled.println("Broche PD3 : High ");
  oled.println("Broche PD4 : High ");
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  delay(500);  
  U0 = PDIV0*analogRead(A0)*VREF/1023.0;
  oled.println("U = " + String(U0) + " V");  
  oled.println("");  
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  oled.println("ok -> fin");  
  while (bouton_ok_appuye == false) {};
  bouton_ok_appuye = false;
}

// Affichage des menus
void afficheMenuPrincipal() {
  oled.clear();
  oled.set2X();
  oled.println("Menu");
  oled.set1X();
  oled.println("");
  oled.println("* Mesure tension");
  oled.println("  Mesure capacite");
  oled.println("  Etalonnage");  
}
void afficheMenuCapacite() {
  oled.clear();
  oled.set2X();
  oled.println("Menu");
  oled.set1X();
  oled.println("");
  oled.println("  Mesure tension");
  oled.println("* Mesure capacite");
  oled.println("  Etalonnage");  
}
void afficheMenuEtalonnage() {
  oled.clear();
  oled.set2X();
  oled.println("Menu");
  oled.set1X();
  oled.println("");
  oled.println("  Mesure tension");
  oled.println("  Mesure capacite");
  oled.println("* Etalonnage");  
}
void afficheMenuNiMh() {
  oled.clear();
  oled.set2X();
  oled.println("Capacite");
  oled.set1X();
  oled.println("");
  oled.println("* Accu Ni-Mh");
  oled.println("  Accu Li-Ion");
}
void afficheMenuLiIon() {
  oled.clear();
  oled.set2X();
  oled.println("Capacite");
  oled.set1X();
  oled.println("");
  oled.println("  Accu Ni-Mh");
  oled.println("* Accu Li-Ion");
}

void setup() {
  
  Serial.begin(9600);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);  
  oled.clear();
  oled.set2X();
  oled.println("Tutoduino");
  oled.set1X();
  oled.println("Apprendre");
  oled.println("l'electronique");
  oled.println("avec un Arduino");
  oled.println("");
  oled.println("ok -> suivant");  
    
  // Positionne la référence de tension 
  // sur la référence interne à 1,1 V
  analogReference(INTERNAL);

  // Les entrees PC1 PC2 et PC3 sont les
  // boutons poussoirs, on y attache des
  // routines d'interruption
  pinMode(brocheBoutonOk, INPUT_PULLUP);
  pinMode(brocheBoutonHaut, INPUT_PULLUP);
  pinMode(brocheBoutonBas, INPUT_PULLUP);
  PCintPort::attachInterrupt(brocheBoutonHaut, isr_up, RISING);
  PCintPort::attachInterrupt(brocheBoutonBas, isr_down, RISING);
  PCintPort::attachInterrupt(brocheBoutonOk, isr_ok, RISING);

  // Les sortie PD3 et PD4 sont les 2 bits 
  // de controle du reseau R-2R
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

  // Attente d'appui sur le bouton ok
  while (bouton_ok_appuye == false) {};
  bouton_ok_appuye = false;
  
  afficheMenuPrincipal();

}

// Boucle principale
void loop() {
  switch (machineEtat) {
    case ME_MENU_TENSION:
      if (bouton_haut_appuye == true) {
        bouton_haut_appuye = false;
      }
      if (bouton_bas_appuye == true) { 
        bouton_bas_appuye = false;
        machineEtat = ME_MENU_CAPACITE;
        afficheMenuCapacite();        
      }
      if (bouton_ok_appuye == true) {
        bouton_ok_appuye = false;
        mesureTension();
        machineEtat = ME_MENU_TENSION;        
        afficheMenuPrincipal();        
      }
      break;
    case ME_MENU_CAPACITE:
      if (bouton_haut_appuye == true) {
        bouton_haut_appuye = false;
        machineEtat = ME_MENU_TENSION;
        afficheMenuPrincipal();        
      }
      if (bouton_bas_appuye == true) { 
        bouton_bas_appuye = false;
        machineEtat = ME_MENU_ETALONNAGE;
        afficheMenuEtalonnage();        
      }
      if (bouton_ok_appuye == true) {
        bouton_ok_appuye = false;        
        machineEtat = ME_MENU_CAPA_NIMH;
        afficheMenuNiMh();        
      }    
      break;
    case ME_MENU_CAPA_NIMH:
      if (bouton_haut_appuye == true) {  
        bouton_haut_appuye = false;
      }
      if (bouton_bas_appuye == true) { 
        bouton_bas_appuye = false;
        machineEtat = ME_MENU_CAPA_LIION;
        afficheMenuLiIon();        
      }
      if (bouton_ok_appuye == true) {
        bouton_ok_appuye = false;        
        mesureCapacite(SEUIL_BAS_TENSION_ACCU_NIMH);
        machineEtat = ME_MENU_TENSION;
        afficheMenuPrincipal();                
      }   
      break; 
    case ME_MENU_CAPA_LIION:
      if (bouton_haut_appuye == true) {  
        bouton_haut_appuye = false;
        afficheMenuNiMh();   
        machineEtat = ME_MENU_CAPA_NIMH;                     
      }
      if (bouton_bas_appuye == true) { 
        bouton_bas_appuye = false;
      }
      if (bouton_ok_appuye == true) {
        bouton_ok_appuye = false;        
        mesureCapacite(SEUIL_BAS_TENSION_ACCU_LIION);
        machineEtat = ME_MENU_TENSION;
        afficheMenuPrincipal();                
      }    
      break;      
    case ME_MENU_ETALONNAGE:
      if (bouton_haut_appuye == true) {
        bouton_haut_appuye = false;
        afficheMenuCapacite();        
        machineEtat = ME_MENU_CAPACITE;
      }
      if (bouton_bas_appuye == true) { 
        bouton_bas_appuye = false;
      }
      if (bouton_ok_appuye == true) {
        bouton_ok_appuye = false;                
        etalonnage();
        machineEtat = ME_MENU_TENSION;        
        afficheMenuPrincipal();                
      }    
      break;
    default:
      break;
  }
  delay(100);
}

// Routine d'interruption du bouton haut
void isr_up() {
  if ((millis() - last_interrupted) > 100)
    bouton_haut_appuye = true;
  last_interrupted = millis();
}

// Routine d'interruption du bouton bas
void isr_down() {
  if ((millis() - last_interrupted) > 100)
    bouton_bas_appuye = true;      
  last_interrupted = millis();
}

// Routine d'interruption du bouton ok 
void isr_ok() {
  if ((millis() - last_interrupted) > 100)
    bouton_ok_appuye = true;
  last_interrupted = millis();
}
  
