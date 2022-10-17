/*
Autor projektu: Mateusz BARTNICKI
Grupa: I0X1N1
Projekt dotyczy układu sterowania silnikiem windy. 
Winda porusza się między trzema następującymi kondygnacjami: 0, 2, 3. 
Stan początkowy – winda ustawia się na 2 kondygnacji.

Założenia ustalone przed prowadzącą laboratoria:
I1 P1 - przycisk wysyłania windy na 0 kondygnację
I2 P2 - przycisk wysyłania windy na 1 kondygnację
I3 P3 - przycisk wysyłania windy na 2 kondygnację
I4 P4 - przycisk wysyłania windy na 3 kondygnację
I5 P5 - czujnik (sensor) obecności windy na 0 kondygnacji
I6 P6 - czujnik (sensor) obecności windy na 1 kondygnacji
I7 P7 - czujnik (sensor) obecności windy na 2 kondygnacji
I8 P8 - czujnik (sensor) obecności windy na 3 kondygnacji

Sterowanie silnikiem windy
F/R, S/S – wejścia sterujące silnikiem połączone z wyjściami PLC
F/R z wyjściem Q1
S/S z wyjściem Q2

Q1   Q2
0 1  stop
0 0  jazda w górę
1 0  jazda w dół
1 1  stan niedozwolony

  Definicja stanów maszyny stanowej:
S_1 - obecność windy na 0 kondygnacji   
S_2 - obecność windy na 2 kondygnacji
S_3 - obecność windy na 3 kondygnacji
S_4 - przejazd windy z 0 na 2 kondygnację
S_5 - przejazd windy z 0 na 3 kondygnację
S_6 - przejazd windy z 2 na 0 kondygnację
S_7 - przejazd windy z 2 na 3 kondygnację
S_8 - przejazd windy z 3 na 0 kondygnację
S_9 - przejazd windy z 3 na 2 kondygnację
S-10 - stan początkowy po włączeniu sterownika, oczekuje na ustalenie obecności windy na danej kondygnacji

  M1  M2  M3  M4
S1  0 0 0 1
S2  0 0 1 0
S3  0 0 1 1
S4  0 1 0 0 
S5  0 1 0 1
S6  0 1 1 1
S7  1 0 0 0
S8  1 0 0 1
S9  1 0 1 0
S10 0 0 0 0
*/

//Zmienne globalne
boolean Q1 = 1;//Forward/Reverse Control Signal Relay 3
boolean Q2 = 0;//Start/Stop Control Signal Relay 4
boolean IP1 = 0;//Odczyt z "Przycisk zadanie 0-go poziomu"
boolean IP2 = 0;//Odczyt z "Przycisk zadanie 2-go poziomu"
boolean IP3 = 0;//Odczyt z "Przycisk zadanie 3-go poziomu"
boolean IS1 = 0;//Odczyt z "Sensor obecnosci kabiny na 0. poziomie"
boolean IS2 = 0;//Odczyt z "Sensor obecnosci kabiny na 2. poziomie"
boolean IS3 = 0;//Odczyt z "Sensor obecnosci kabiny na 3. poziomie"
boolean M1 = 0; //kod stanu maszyny stanowej
boolean M2 = 0; //kod stanu maszyny stanowej
boolean M3 = 0; //kod stanu maszyny stanowej
boolean M4 = 0; //kod stanu maszyny stanowej

void setup() {
  // put your setup code here, to run once:
  #define Button1Pin A0 //nazwa pinu dla "Przycisk żądanie 0-go poziomu"
  #define Button2Pin A1 //nazwa pinu dla "Przycisk żądanie 1-go poziomu"
  #define Button3Pin A2 //nazwa pinu dla "Przycisk żądanie 2-go poziomu"
  #define Button4Pin A3 //nazwa pinu dla "Przycisk żądanie 3-go poziomu"
  #define Sensor1Pin A4 //nazwa pinu dla "Sensor obecności kabiny na 0-wym poziomie"
  #define Sensor2Pin A5 //nazwa pinu dla "Sensor obecności kabiny na 1-ym poziomie"
  #define Sensor3Pin 12 //nazwa pinu dla "Sensor obecności kabiny na 2-im poziomie"
  #define Sensor4Pin 13 //nazwa pinu dla "Sensor obecności kabiny na 3-im poziomie"
  #define OutputQ1Pin 5 //nazwa pinu dla Forward/Reverse Signal (bit kierunku)
  #define OutputQ2Pin 4 //nazwa pinu dla Start/Stop Signal (bit stopu)

  pinMode(Button1Pin, INPUT);
  pinMode(Button2Pin, INPUT);     //nie bedzie wykorzystywane, winda ma poruszac sie tylko po 0, 2 i 3 kondygnacji
  pinMode(Button3Pin, INPUT);
  pinMode(Button4Pin, INPUT);
  pinMode(Sensor1Pin, INPUT);
  pinMode(Sensor2Pin, INPUT);     //nie bedzie wykorzystywane, winda ma poruszac sie tylko po 0, 2 i 3 kondygnacji
  pinMode(Sensor3Pin, INPUT);
  pinMode(Sensor4Pin, INPUT);
  pinMode(OutputQ1Pin, OUTPUT);
  pinMode(OutputQ2Pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Koniec wykonywania Setup");
}

void odczytWejsc()
{
  IP1 = digitalRead(Button1Pin);
  IP2 = digitalRead(Button3Pin);
  IP3 = digitalRead(Button4Pin);
  IS1 = digitalRead(Sensor1Pin);
  IS2 = digitalRead(Sensor3Pin);
  IS3 = digitalRead(Sensor4Pin);
}

void funkcjaPrzejscia()
{
  //zmienne pomocnicze do zapamietania nowych, obliczonych stanow flagi
  boolean M1prim = 0; 
  boolean M2prim = 0; 
  boolean M3prim = 0; 
  boolean M4prim = 0; 
  
  //Obliczenie wartosci funkcji przejsc stanow (flag) - ZGODNIE Z FUNKCJA PRZEJSC PO MINIMALIZACJI
  M1prim = (!M1 & !M2 & M3 & M4 & (IP1 | IP2)) | (!M2 & !M4 & ((IP3 & !M1 & M3) | (!IS3 & M1 & !M3))) | (M1 & !M2 & ((!IS1 & !M3 & M4) | (!IS2 & M3 & !M4))) | (!IS1 & !IS2 & IS3 & !M1 & !M2 & !M3 & !M4);
  M2prim = (!M1 & !M2 & !M3 & M4 & (IP2 | IP3)) | (!M1 & M2 & !M3 & ((!IS2 & !M4) | (!IS3 & M4))) | (!M1 & M3 & ((IP1 & !M2 & !M4) & (!IS1 & M2 & M4))) | (IS1 & !IS2 & !IS3 & !M1 & !M2 & !M3 & !M4);
  M3prim = (!M1 & !M2 & M3 & !M4 & (!IP3 | IP1)) | (!M1 & !M2 & M3 & M4 & (!IP1 | IP2)) | (!M1 & M2 & ((IS2 & !M3 & !M4) | (!IS1 & M3 & M4))) | (IS3 & !M3 & ((!M1 & M2 & M4) | (M1 & !M2 & !M4))) | (!M2 & !M4 & M1 & M3) | ((!M1 & !M2 & !M3 & !M4 & !IS1) & ((IS2 & !IS3) | (IS3 | !IS2)));
  M4prim = (!M1 & !M2 & !M3 & M4 & (!IP2 | IP3)) | (!M1 & !M2 & M3 & (IP1 | (!IP2 & M4))) | (M1 & !M2 & !M3 & (IS3 | M4)) | (!M1 & M2 & M4);

  //Przepisanie "nowych" wartosci do "starych" 
  M1 = M1prim;
  M2 = M2prim;
  M3 = M3prim;
  M4 = M4prim;
}

void funkcjaWyjscia() 
{
  Q1 = (!M1 & M2 & M3 & M4) | (M1 & !M2 & ((!M3 & M4) | (M3 & !M4)));
  Q2 = (!M1 & !M2 & (M4 | M3)); 
 }

void zapisWyjscia()
{
  digitalWrite(OutputQ1Pin,Q1);
  digitalWrite(OutputQ2Pin,Q2);
}

void raport()
{
  Serial.println("Odczyt wartosci podawanych na wejscie sterownika  ");
  Serial.print(IP1);
  Serial.print(IP2);
  Serial.print(IP3);
  Serial.print(IS1);
  Serial.print(IS2);
  Serial.print(IS3);
  Serial.println();  
  Serial.println("Odczyt stanu  ");
  Serial.print(M1);
  Serial.print(M2);
  Serial.print(M3);
  Serial.print(M4);
  Serial.println();  
  Serial.println("Odczyt wyjsc  ");
  Serial.print(Q1);
  Serial.print(Q2);
  Serial.println(); 
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  odczytWejsc();
  funkcjaPrzejscia();
  funkcjaWyjscia();
  zapisWyjscia();
  raport();
}
