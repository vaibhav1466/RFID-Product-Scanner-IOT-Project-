#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

// LCD pins
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
 int buzzerPin = 8;

// RFID pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// Product database structure
struct Product {
  String uid;
  String name;
  float price;
  int quantity; // Will be either 0 or 1 in this version
};

// Define your products here (UID should match your RFID cards)
const int MAX_PRODUCTS = 10; // Maximum number of different products
Product products[MAX_PRODUCTS] = {
  {"B99B98C1", "Milk Gold", 68.00, 0},
  {"99FBFFC1", "Bread", 25.00, 0},
  {"F7EA9B5F", "Mosambi Juice", 20.00, 0},
  {"E7949F5F", "Ganne Juice", 25.00, 0},
  // Add more products as needed
};

int productCount = 4; // Update this to match the number of products you've defined
float totalAmount = 0.00;

void setup() {
  Serial.begin(9600);
  SPI.begin();          // Init SPI bus
  rfid.PCD_Init();      // Init MFRC522
  lcd.begin(16, 2);     // Init LCD
  pinMode(buzzerPin, OUTPUT);
  displayWelcomeMessage();
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Get UID of the scanned card
  String uid = getRFIDUID();
 
  // Find the product in our database
  int productIndex = findProductIndex(uid);
 
  if (productIndex != -1) {
    // Toggle product quantity (add or remove)
    if (products[productIndex].quantity == 0) {
      // Add product
      products[productIndex].quantity = 1;
      totalAmount += products[productIndex].price;
      displayAddedMessage(productIndex);
    } else {
      // Remove product
      products[productIndex].quantity = 0;
      totalAmount -= products[productIndex].price;
      displayRemovedMessage(productIndex);
    }
   
    playBeep();
    printReceipt();
  } else {
    // Unknown card
    displayUnknownCard(uid);
    playBeep();
  }

  // Halt PICC
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
 
  delay(1000); // Wait 1 second before next scan
  displayWelcomeMessage();
}

String getRFIDUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println("Scanned UID: " + uid);
  return uid;
}

int findProductIndex(String uid) {
  for (int i = 0; i < productCount; i++) {
    if (products[i].uid == uid) {
      return i;
    }
  }
  return -1; // Not found
}

void displayAddedMessage(int index) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Added:");
  lcd.print(products[index].name);
  lcd.setCursor(0, 1);
  lcd.print("Price: Rs.");
  lcd.print(products[index].price);
}

void displayRemovedMessage(int index) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Removed:");
  lcd.print(products[index].name);
  lcd.setCursor(0, 1);
  lcd.print("Refund: Rs.");
  lcd.print(products[index].price);
}

void displayUnknownCard(String uid) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unknown product");
  lcd.setCursor(0, 1);
  lcd.print("UID:");
  lcd.print(uid.substring(0, 8)); // Display first part of UID
}

void displayWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan product");
  lcd.setCursor(0, 1);
  lcd.print("Total: Rs.");
  lcd.print(totalAmount);
}

void playBeep() {
  Serial.println("Beeppppp");
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
}

void printReceipt() {
  Serial.println("\n----- Current Receipt -----");
  bool hasItems = false;
 
  for (int i = 0; i < productCount; i++) {
    if (products[i].quantity > 0) {
      hasItems = true;
      Serial.print(products[i].name);
      Serial.print("\tRs.");
      Serial.println(products[i].price);
    }
  }
 
  if (!hasItems) {
    Serial.println("Cart is empty");
  }
 
  Serial.print("TOTAL: Rs.");
  Serial.println(totalAmount);
  Serial.println("---------------------------\n");
}


















