 
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <SPI.h>
#include <SD.h>

/*FIREBASE DEFINES*/
#define FIREBASE_HOST "fire-esp-91105.firebaseio.com"
#define FIREBASE_STORAGE "gs://fire-esp-91105.appspot.com"
#define FIREBASE_AUTH "6KzcOJSZKMEjzs1HDLZLo0SGdy7RTTn4iUbywcK0"
#define WIFI_SSID "Valencia2Y100pro"
#define WIFI_PASSWORD NULL
#define PATH_FINGERID "/FingerID"
#define PATH_USERNAME "/UserName"
#define RET_USERS "Users"
#define RET_SONGS "Songs"
#define PATH_SONGLIST "/Songs"
#define ERRORNOUSER "NoUserFound"
#define ERRORNOSONGS "NoSongsFound"
#define ERRORDELUSER 0
#define ERRORNONEWUSER 0

/*SDCARD DEFINES*/
#define SS_PIN SS
#define BufferSize 1024
File myFile;
unsigned char FirstHalfBuffer[BufferSize];
unsigned char SecondHalfBuffer[BufferSize];
uint8_t Flag_Buffer = 0;

union SizeFile {
  int SizeOfFile;
  uint8_t SizeOfFileBytes[4];
};

/*SERIAL DEFINES*/
#define BAUDRATE 115200

/**********************************************************************************************************
 * Function Name: SongList
 * Description: Returns the Song List From the Database
 * Return: Returns a string with the list of songs from Firebase
 **********************************************************************************************************/

String SongList()
{
  return Firebase.getString(RET_SONGS);
}

/**********************************************************************************************************
 * Function Name: UserSongList
 * Description: Returns a string with the list of songs the user has access to
 * Parameter: Value must be an integer between 0-999 which will be one of the users stored in the database
 * Return: Returns string
 **********************************************************************************************************/

String UserSongList(int value)
{
  String ListOfSongs;
  String path = "/";
  String ID = String(value); //turns the value into a string
     
//  Serial.print("Search for user: ");
//  Serial.println(value);
 
  path.concat(ID);
  path.concat(PATH_SONGLIST);
  ListOfSongs = Firebase.getString(path);
//  Serial.print("Path: ");
//  Serial.println(path);
  if(!Firebase.success())
  {
  ListOfSongs = ERRORNOSONGS;
  }
  
  return ListOfSongs;
}

/**********************************************************************************************************
 * Function Name: SetFingerPrint
 * Description: Sets on the new user in the Firebase its finger printt id
 * Parameter: Value must be an integer between 0-999 which will be one of the users stored in the database
 **********************************************************************************************************/
void SetFingerPrint(int UserID)
{
  String path = "/";
  path.concat(String(UserID));
  path.concat(PATH_FINGERID);
  Firebase.setInt(path, UserID);
}
/**********************************************************************************************************
 * Function Name: DelUser
 * Description: Removes a path on the firebase for the designated user
 * Parameter: Value must be an integer between 0-999 which will be one of the users stored in the database
 * Return: Returns 1 on success
 **********************************************************************************************************/
int DelUser(int UserID)
{
  String path = "/";
  int UserIDPos = 0;
  int UserSize = 0;
  String UserList;
  String UserString = String(UserID);
  UserSize = UserString.length();
  UserList = Firebase.getString("Users");
  UserIDPos = UserList.indexOf(String(UserID));
  UserList.remove(UserIDPos, UserSize + 4);
  Firebase.setString("Users", UserList);
  path.concat(String(UserID));
  Firebase.remove(path);
  return 1;
}
/**********************************************************************************************************
 * Function Name: NewUser
 * Description: Returns the UserID without a FingerID
 * Return: Returns 0 in case of erro or and int of the User ID
 **********************************************************************************************************/

int NewUser()
{
  String UserList;
  String path = "/";
  int LastSeparator, FingerID, Size, UserID = 0;
  UserList = Firebase.getString("Users");
  LastSeparator = UserList.lastIndexOf(",");
  Size = UserList.length();
  path.concat(UserList.substring(LastSeparator + 1, Size));
  path.concat(PATH_FINGERID);
  FingerID = Firebase.getInt(path);
  UserID = UserList.substring(LastSeparator + 1, Size).toInt();

  if(FingerID == 500)
  {
    return UserID;
  }
  else return ERRORNONEWUSER;
}

/**********************************************************************************************************
 * Function Name: FindDelUser
 * Description: Returns the user finger id to be deleted
 * Return: Returns an int with users id
 **********************************************************************************************************
 */
int FindDelUser()
{
  String UserList = ",";
  String path = "/";
  int DelPos = 0, SeparatorBefore = 0,SeparatorAfter = 0 ,FingerID, InitialPos = 0;

//  Serial.println("Delete User? Debugging");
  
  UserList.concat(Firebase.getString("Users"));  //Starts by getting all the users
  UserList.concat(",");
  DelPos = UserList.indexOf("del");

  if(DelPos != -1)
  {
    while(!(SeparatorBefore < DelPos && SeparatorAfter > DelPos))
    {
      SeparatorBefore = UserList.indexOf(",", InitialPos);
      SeparatorAfter = UserList.indexOf(",", SeparatorBefore + 1);
      InitialPos = SeparatorBefore + 1;
    }
  }else return ERRORDELUSER;

  if(SeparatorBefore < DelPos)
  {
    SeparatorBefore++;
  } else SeparatorBefore = 0;
  
  path.concat(UserList.substring(SeparatorBefore, DelPos));
  path.concat(PATH_FINGERID);

  FingerID = Firebase.getInt(path);

  return FingerID;
}

/**********************************************************************************************************
 * Function Name: UserName
 * Description: Returns the user name associated with the value it takes as a parameter
 * Parameter: Value must be an integer between 0-999 which will be one of the users stored in the database
 * Return: Returns a string with users name or an error message if use not found
 **********************************************************************************************************
 */

String UserName(int value)
{
  String NameOfUser;
  String path = "/";
  String ID = String(value); //turns the value into a string
  
  path.concat(ID);
  path.concat(PATH_USERNAME);
  NameOfUser = Firebase.getString(path);
  if(!Firebase.success())
  {
  NameOfUser = ERRORNOUSER;
  }
   
  return NameOfUser;
}

/**********************************************************************************************************
 * Function Name: RetSong
 * Description: Sends throw the uart 1kbytes of information from 
 * Parameter: Value must be an integer between 0-999 which will be one of the users stored in the database
 * Return: Returns 1 on success and 0 if fails
 **********************************************************************************************************/

int RetSong(String str)
{
  String SongName = str;
  uint8_t Flag_First = 0;
  int BuffSize = 1;
  uint8_t Buffer[BuffSize];
  uint8_t Chars[78];
  uint8_t index = 0;
  String FileData;
  int counter = 78;
  SizeFile Data;
  SongName.remove(SongName.indexOf("$"));
  SongName.concat(".wav");
  delay(1000);     
  if(SD.exists(SongName))
  {
      //Serial.println("File exists.");
      myFile = SD.open(SongName, FILE_READ);
 //     Serial.println("File Data:");
      while(counter--)
      {
       Chars[index] = myFile.read();
       index++;
      } // it print the data correctly until the part of the file that has been corrupted
      Data.SizeOfFileBytes[3] = Chars[7];
      Data.SizeOfFileBytes[2] = Chars[6];
      Data.SizeOfFileBytes[1] = Chars[5];
      Data.SizeOfFileBytes[0] = Chars[4];
      Serial.print(Data.SizeOfFile);
      Serial.write("$$");
      
      if(myFile.available())
      {
        myFile.readBytes(FirstHalfBuffer, BufferSize);
        delay(100);
        myFile.readBytes(SecondHalfBuffer, BufferSize);
        delay(1000);
        Serial.write(FirstHalfBuffer, BufferSize);
        //Serial.write("$$");
      }
      while(myFile.available() && Serial.readBytes(Buffer,BuffSize))
      {
        if(!Flag_Buffer)
        {
          Flag_Buffer = 1;
          Serial.write(FirstHalfBuffer, BufferSize);
          myFile.readBytes(FirstHalfBuffer, BufferSize);
          //Serial.write("$$");
        }
        else if(Flag_Buffer)
        {
          Flag_Buffer = 0;
          Serial.write(SecondHalfBuffer, BufferSize);
          myFile.readBytes(SecondHalfBuffer, BufferSize);
          //Serial.write("$$");
        }
      }
      myFile.close();
  } else {
    Serial.println("File does not exist");
    return 0;//
  }
Serial.print("MO$$");
return 1;
}

/**********************************************************************************************************
 * Function Name: Setup
 * Description: Configures Serial Interface, Wi-Fi Settings, Firebase Settings and SD Card
 **********************************************************************************************************/

void setup() {

Serial.begin(BAUDRATE);

Serial.setTimeout(1000);

Serial.print("Connecting to: ");
Serial.println(WIFI_SSID);

WiFi.mode(WIFI_STA);

WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

Serial.println();
Serial.println("Connecting...");

  while (WiFi.status() != WL_CONNECTED) 
  {
//    Serial.print(".");
    delay(500);
  }

Serial.println("Wi-Fi Connected");
Serial.print("IP Address: ");
Serial.println(WiFi.localIP());

Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //Firebase to get info

Serial.println("Initializing SD card...");

if (!SD.begin(SS_PIN)){
Serial.println("Initialization Failed!");
return;
} Serial.println("Initialization Done.");

}

/**********************************************************************************************************
 * Function Name: loop
 * Description: Sends throw the uart 1kbytes of information from 
 **********************************************************************************************************/

void loop() {

String NameOfUser; //Name of the User
String SongListOfUser; //List of songs
String FirebaseSongList = ","; 
String SongName; //Song name in string
String Temp; //Temp value for calculations
String SerialBuffer;
int SerialBufferSize;
int UserTemp; //Temp value for user
int SongTemp; //Temp values for songs
int DelTemp; // Temp values for delete users
int NewTemp; //Temp value for new users
int UserID; //User number
int SongID; //Song number
int DelUserID; //User to be deleted after confirmation
int FingerUserID; //Number of the finger print

int RetNewUser; //New User number
int RetDelUser; //Delete User number
int UserForFinger; //Number of user for the finger

int FirstPosition = 0; //Counters
int NextPosition = 0; //Counters

uint16_t DelNewCounter = 10000; //Counters
int FlagBuffer = 0; //Flag to know that the buffer has new information

// available returns the number of bytes available to read:
 while (Serial.available() > 0) {
   // read the incoming byte: int and cast it into a char
   SerialBuffer.concat(char(Serial.read()));
   FlagBuffer = 1;
 }

  if (FlagBuffer == 1)
  {
      FlagBuffer = 0;
      if(SerialBuffer.indexOf("U")>=0) //Choose User
      {
        SerialBufferSize = SerialBuffer.length();
        UserTemp = SerialBuffer.indexOf("U");
        Temp = SerialBuffer.substring(UserTemp + 1, SerialBufferSize);
        UserID = Temp.toInt();
        NameOfUser = UserName(UserID);
        //Serial.print("User Name: ");
        NameOfUser.concat("$$");
        Serial.print(NameOfUser);
        //Serial.write("$$");
        SongListOfUser = UserSongList(UserID);
        //Serial.print("Song List: ");
        SongListOfUser.concat("$$");
        delay(10);
        Serial.print(SongListOfUser);
        //Serial.write("$$");
      }
      else if(SerialBuffer.indexOf("S")>=0) //Choose Song
      {
        SerialBufferSize = SerialBuffer.length();
        SongTemp = SerialBuffer.indexOf("S");
        Temp = SerialBuffer.substring(SongTemp + 1, SerialBufferSize);
        SongID = Temp.toInt();
        FirebaseSongList.concat(SongList());
        FirebaseSongList.concat(",");
        //CICLE TO SEE WHERE IS THE SONG
        for(int i=SongID;i>0;i--)
        {
          FirstPosition = NextPosition;
          NextPosition = FirebaseSongList.indexOf(",",FirstPosition + 1);
        }
        SongName = FirebaseSongList.substring(FirstPosition + 1, NextPosition);
        SongName.concat("$$");
        Serial.print(SongName);
        //Serial.write("$$");
        RetSong(SongName);
      }
      else if(SerialBuffer.indexOf("D")>=0) //Send more data
      {
        SerialBufferSize = SerialBuffer.length();
        DelTemp = SerialBuffer.indexOf("D");
        Temp = SerialBuffer.substring(DelTemp + 1, SerialBufferSize);
        DelUserID = Temp.toInt();
      }
      else if(SerialBuffer.indexOf("N")>=0) //Send more data
      {
        SerialBufferSize = SerialBuffer.length();
        NewTemp = SerialBuffer.indexOf("D");
        Temp = SerialBuffer.substring(NewTemp + 1, SerialBufferSize);
        FingerUserID = Temp.toInt();
        SetFingerPrint(FingerUserID);
      }

     
  }
  DelNewCounter--;
  if(DelNewCounter == 0)
  {
    DelNewCounter = 10000;
    RetDelUser = FindDelUser(); //FIND THE USER TO DELETE
    if(RetDelUser != 0)
    {
      Serial.print("D");
      Serial.print(RetDelUser);
      Serial.write("$$");
    }
    RetNewUser = NewUser(); // RETURN IT TRUE
    if(RetNewUser != 0)
    {
      Serial.print("N");
      Serial.print(RetNewUser);
      Serial.write("$$");
    }
  }

 /*CLEAN UP AREA BEFORE ENDING THE LOOP*/
 SerialBuffer = "";
 SerialBufferSize = 0;
 FirebaseSongList = ",";
 FirstPosition = 0;
 NextPosition = 0;
}
