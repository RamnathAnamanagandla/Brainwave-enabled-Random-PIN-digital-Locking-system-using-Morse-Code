#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
SoftwareSerial bt(3, 2);
File myFile;
#define   BAUDRATE           57600
#define   Theshold_Eyeblink  110
#define   EEG_AVG            70

String originalPassword = "";
long payloadDataS[5] = {0};
long payloadDataB[32] = {0};
String pswd="";
int state=0;
byte checksum=0,generatedchecksum=0;
unsigned int Raw_data,Poorquality,Plength,Eye_Enable=0,On_Flag=0,Off_Flag=1 ;
unsigned int j,n=0;
long Temp,Avg_Raw,Temp_Avg;


char binDecode(String input){

   if(input.equals("0000"))
    return '0';
   if(input.equals("0001"))
    return '1';
   if(input.equals("0010"))
    return '2';
   if(input.equals("0011"))
    return '3';
   if(input.equals("0100"))
    return '4';
   if(input.equals("0101"))
    return '5';
   if(input.equals("0110"))
    return '6';
   if(input.equals("0111"))
    return '7';
   if(input.equals("1000"))
    return '8';
   if(input.equals("1001"))
    return '9';
   
    
    
}

char morseDecode(String input){

   if(input.equals("-----"))
    return '0';
   if(input.equals(".----"))
    return '1';
   if(input.equals("..---"))
    return '2';
   if(input.equals("...--"))
    return '3';
   if(input.equals("....-"))
    return '4';
   if(input.equals("....."))
    return '5';
   if(input.equals("-...."))
    return '6';
   if(input.equals("--..."))
    return '7';
   if(input.equals("---.."))
    return '8';
   if(input.equals("----."))
    return '9';
   
    
    
}

String morseEncode(int input){

   if(input==0)
    return "-----";
   if(input==1)
    return ".----";
   if(input==2)
    return "..---";
   if(input==3)
    return "...--";
   if(input==4)
    return "....-";
   if(input==5)
    return ".....";
   if(input==6)
    return "-....";
   if(input==7)
    return "--...";
   if(input==8)
    return "---..";
   if(input==9)
    return "----.";
   
    
    
}

  
void gotoMyFun(String input){


  if(input.charAt(1) == '1'){

int y =0;
myFile = SD.open("PIN.txt");
String curpin= myFile.readString();
myFile.close();
originalPassword="";
for(int i = 0; i < 4; i++){

      char ch1 = morseDecode(curpin.substring(y, y + 5));
      originalPassword += ch1;
 
      y += 5;
}

   
    if(state==0)
    {
    int z = 2;
  
  String tempPassword = "";
  for(int i = 0; i < 4; i++){

      char ch = binDecode(input.substring(z, z + 4));
      tempPassword += ch;
 
      z += 4;
  }

  if(originalPassword.equals(tempPassword)){
   lcd.clear();
   lcd.setCursor(0,0);
  lcd.print("CORRECT PIN:"+ tempPassword);
  lcd.setCursor(0,1);
  lcd.print("TURN OFF BCI" );
  state=1;
  digitalWrite(A0,HIGH);
  digitalWrite(A5,LOW);

  delay(10000);
  }
  else{

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WRONG PIN: " + tempPassword);
    lcd.setCursor(0,1);
    lcd.print("TRY AGAIN" );
    digitalWrite(A5,HIGH);
    delay(3000);
  }
  }
  else{
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("ALREADY OPENED" );
     delay(10000);
    }
  }

  if(input.charAt(1) == '0')
  {
   if(state==1)
   {
    String s = "";
    s = s + morseEncode(random(10))  + morseEncode(random(10))  + morseEncode(random(10))  + morseEncode(random(10)) ;
    originalPassword = s ;
    state=0;
    lcd.clear();
   lcd.setCursor(0,0);
    lcd.print("NEW PIN IS");
    delay(2000);
    lcd.clear();
   lcd.setCursor(0,0);
    lcd.print(originalPassword);
    if(originalPassword.length()>16)
    {
      lcd.setCursor(0,1);
  lcd.print(originalPassword.substring(16,20));
     }
     SD.remove("PIN.txt");
   myFile = SD.open("PIN.txt", FILE_WRITE);
   myFile.println(originalPassword);
   myFile.close();
     delay(20000);
     lcd.clear();
     lcd.setCursor(0,0);
  lcd.print("TURN OFF BCI" );
  delay(2000);
  digitalWrite(A0,LOW);
    delay(5000);
  }
  else{
  lcd.clear();
  lcd.setCursor(0,0);  
  lcd.print("ALREADY CLOSED" );
  delay(10000);
  
  }
   
  } 
}
 void setup() 
 {          
  randomSeed(analogRead(0));
    pinMode(A0,OUTPUT);
    digitalWrite(A0,LOW);
    pinMode(A5,OUTPUT);
    digitalWrite(A5,LOW);
   lcd.begin(16, 2);
  lcd.clear();
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("INITIALIZING SD");
  lcd.setCursor(0,1);
  if (!SD.begin()) {
    delay(2000);
    lcd.print("INIT FAILED");
    delay(2000);
    return;
  }
  delay(2000);
  lcd.print("INIT DONE");
   bt.begin(BAUDRATE);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  delay(2000);
 }
 

 byte ReadOneByte()           // One Byte Read Function
 {
   int ByteRead;
   while(!bt.available());
  ByteRead = bt.read();
   return ByteRead;
 }

 void loop()                     // Main Function
 {

   if(ReadOneByte() == 170)        // AA 1 st Sync data
   {
     if(ReadOneByte() == 170)      // AA 2 st Sync data
     {
       Plength = ReadOneByte();
       if(Plength == 4)   // Small Packet
       { 
         Small_Packet ();
       }
       else if(Plength == 32)   // Big Packet
       { 
         Big_Packet ();
       }
     }
   }         
 }
 
 void Small_Packet ()
 {
   generatedchecksum = 0;
   for(int i = 0; i < Plength; i++) 
   {  
     payloadDataS[i]     = ReadOneByte();      //Read payload into memory
     generatedchecksum  += payloadDataS[i] ;
   }
   generatedchecksum = 255 - generatedchecksum;
   checksum  = ReadOneByte();
   if(checksum == generatedchecksum)        // Varify Checksum
   {  
     if (j<512)
     {
        Raw_data  = ((payloadDataS[2] <<8)| payloadDataS[3]);
       if(Raw_data&0xF000)
       { 
         Raw_data = (((~Raw_data)&0xFFF)+1); 
       }
       else
       { 
         Raw_data = (Raw_data&0xFFF); 
       }
       Temp += Raw_data;
       j++; 
     }
     else 
     { 
       Onesec_Rawval_Fun ();
     }
   }
 }
 
 void Big_Packet()
 {
   generatedchecksum = 0;
   for(int i = 0; i < Plength; i++) 
   {  
     payloadDataB[i]     = ReadOneByte();      //Read payload into memory
     generatedchecksum  += payloadDataB[i] ;
   }
   generatedchecksum = 255 - generatedchecksum;
   checksum  = ReadOneByte();
   if(checksum == generatedchecksum)        // Varify Checksum
   {
     Poorquality = payloadDataB[1];
     if (Poorquality==0 )
     {
       Eye_Enable = 1; 
     }
     else
     { 
       Eye_Enable = 0;
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("POOR QUALITY");
     }
   }
 }
 
 void Onesec_Rawval_Fun ()
 {
   Avg_Raw = Temp/512;
   if (On_Flag==0 && Off_Flag==1)
   {
     if (n<3)
     {
       Temp_Avg += Avg_Raw;
       n++;
     }
     else
     {
       Temp_Avg = Temp_Avg/3;
       if (Temp_Avg<EEG_AVG) 
       {
         On_Flag=1;Off_Flag=0;
       }
       n=0;Temp_Avg=0;
     }  
   }         
   Eye_Blink ();
   j=0;
   Temp=0; 
 }


 
 void Eye_Blink ()
 {
   if (Eye_Enable)          
   {
     if (On_Flag==1 && Off_Flag==0) 
     { 
       if ((Avg_Raw>Theshold_Eyeblink) && (Avg_Raw<350))
       { 
         pswd+='1';
       }
       else
       {
         if (Avg_Raw>350)
         {
           On_Flag==0;Off_Flag==1;
         }
         pswd+='0';
       }
     }
   }
   if(pswd.length()>=1 and pswd[0]=='1')    
   {
   if(pswd.length()<=18)
   { 
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print(pswd); 
   if(pswd.length()>16)
   {
    lcd.setCursor(0,1);
   lcd.print(pswd.substring(16,18));
   }
   if(pswd.length()==18 and pswd[1]=='1')
   {
   delay(2000); 
   gotoMyFun(pswd);
   pswd="";
   }
   if(pswd.length()==2 and pswd[1]=='0')
   {
   delay(2000); 
   gotoMyFun(pswd);
   pswd="";
   }
   }
   }
   else if(pswd.length()==1 and  pswd[0]=='0')
   {
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("Enter the Input"); 
   pswd="";
   }
 }
