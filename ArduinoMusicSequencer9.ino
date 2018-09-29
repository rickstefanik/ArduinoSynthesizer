// Set up LCD.
#include <LiquidCrystal.h>
LiquidCrystal lcd(11, 12, 4, 5, 6, 7);

// Digital Pins
const int lengthButton = 2;
const int tempoButton = 3;
const int playButton = 8;
const int tempoLED = 9;
const int soundPiezo = 10;
const int writeButton = 13;

// Analog Pins
const int adjustPot = 0;
const int pitchPot = 1;

// soundButton could be a Digital Input, but 
// it is an Analog Pin since all other
// Digital Pins are used.
const int soundButton = 3;


// Array of names of Notes (to be displayed on LCD
// CLR and REST are special commands and hence have special "frequencies".
const char* Notes[] =  {"CLR", "REST", "A1", "A#1", "B1", "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2", "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3", "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4", "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5"};
// Parallel Array of Frequencies
const int Frequencies[] = {-1,     0,   55,    58,   62,   65,    69,   73,    78,   82,   87,    92,   98,   104,  110,   117,  123,  131,   139,  147,   156,  165,  175,   185,  196,   208,  220,   233,  247,  262,   277,  294,   311,  330,  349,   370,  392,   415,  440,   466,  494,  523,   554,  587,   622,  659,  698,   740,  784,   831,  880};

const int minPatternLength = 4;
const int maxPatternLength = 64;
volatile int patternLength = 20;

// This array will hold the values of the Frequencies for beat in
// the pattern.  In the setup function, each of its values will be 
// set to 0, to ensure C every note is initially a REST.
volatile int pattern[maxPatternLength];

// This variable will change based on the desired tempo.
// The lower this value, the faster the tempo. 
volatile int delayTime = 250; 

// When the user plays a note simply to hear the tone,
// (not actually write it) the note plays for this many
// many milliseconds.  
const int testNoteLength = 250;


// These are simple initial values for certain variables,
// used so the LCD has something to display immediatley on startup.
const int initialPitchValue = 0;
const int initialLocationValue = 0;
const int initialLocationFrequencyValue = 0;

// This two values essentially set the minimum 
// and maximum possible tempos for the pattern.
const int minDelayValue = 150;
const int maxDelayValue = 400;

void setup() {

// Ensures that all values of pattern are initally set to zero.  
  setToZeros(0);

//Startup Song
  pattern[0] = 330; 
  pattern[1] = 440; 
  pattern[2] = 440; 
  pattern[3] = 392; 
  pattern[4] = 330; 
  pattern[5] = 440;
  pattern[6] = 330; 
  pattern[7] = 330; 
  pattern[8] = 0; 
  pattern[9] = 262; 
  pattern[10] = 294; 
  pattern[11] = 330; 
  pattern[12] = 0; 
  pattern[13] = 659; 
  pattern[14] = 587; 
  pattern[15] = 523; 
  pattern[16] = 587; 
  pattern[17] = 0; 
  pattern[18] = 523; 
  pattern[19] = 523; 

  lcd.begin(16, 2);
   
  pinMode(playButton, INPUT);
  pinMode(tempoLED, OUTPUT);
  pinMode(lengthButton, INPUT);
  pinMode(tempoButton, INPUT);
  pinMode(soundPiezo, OUTPUT);
  pinMode(writeButton, INPUT);


  // The changeTempo and changeLength features are triggered
  // by using iterrupts.
  attachInterrupt(tempoButton-2, changeTempo, RISING);
  attachInterrupt(lengthButton-2, changeLength, RISING);

  displayCurrentValues();


  // Sets the tempoLED to the appropriate brightness based on tempo (delayTime).
  // The shorter the delay time, the brighter the LED.
  analogWrite(tempoLED, map(delayTime, minDelayValue, maxDelayValue, 255, 0)); 
}

void loop() {

  // These reference variables keep track of what the values of these variables 
  // are at the beginning of the loop.  Later in the loop, the program checks if 
  // any of these values have changed from what they originally were.  If they have
  // changed, then the LCD is refreshed.  They are set equal to the dummy initial values
  // to avoid issues on the first time through the loop.  These variables are declared as 
  // static, so this variable declaration only occurs once, and not every time through the loop.
  static int referencePitchValue = initialPitchValue;
  static int referenceLocationValue = initialLocationValue;
  static int referenceLocationFrequencyValue = initialLocationFrequencyValue;


  // pitchValue corresponds to an index in the Frequencies array.
  int pitchVoltage = analogRead(pitchPot);
  int pitchValue = map(pitchVoltage, 0, 1023, 0, ((sizeof(Frequencies)) / (sizeof(int))));
  
  // Some issues with flickering occured when pitchVoltage was mapped to its maximum possible value,
  // so the maximum number it could be mapped to is actually one higher than desired.  If this value 
  // is obtained, the program immediately decreases it by one.  In this way, the right number of values
  // are effectively mapped and the flickering is avoided.
  if (pitchValue == (sizeof(Frequencies)) / (sizeof(int)))
  {
      pitchValue = ((sizeof(Frequencies)) / (sizeof(int)) - 1);
  }
  
  
  
  // If the soundButton is hit, the piezo will play the current note.
  // Since soundButton had to be used as an analog input (since all other
  // digital inputs were taken), the if statement uses analogRead.  If soundButton
  // were in a digital input, the statement would read: if (digitalRead(soundButton) == HIGH)
  
  if (analogRead(soundButton) > (1023/2))
  {

    // When pitchValue is equal to 0, it corresponds to CLR.
    // When pitchValue is equal to 1, it corresponds to REST.
    // Since these values do not have actual sounds corresponding to them,
    // no sounds should be played when the soundButton is hit.  Hence the
    // following if statement.
    if (pitchValue > 1)
    {
    tone(soundPiezo, Frequencies[pitchValue], testNoteLength); 
    }
  }

  // Similar to pitchVoltage, to stop the screen from flickering, the map function
  // and following if statement are written in this way.  
  int locationVoltage = analogRead(adjustPot);
  int locationValue = map(locationVoltage, 0, 1023, 0, patternLength);

  if (locationValue == patternLength)
  {
      locationValue = patternLength - 1;
  }

  // If the user hits the write button, the current note will be placed at the current location
  if (digitalRead(writeButton) == HIGH)
  {

    // If user hits the write button when CLR is on the screen (when Frequencies[pitchValue] equals -1),
    // then the entire pattern will be cleared.  Useful when wanting to start a new pattern.   
    if (Frequencies[pitchValue] == -1)
    {
      setToZeros(0);
    }

    // Otherwise, the current note is simply written into the current location.
    // If a note is already in that location, it is overwritten with the
    // new note.
    else 
    {
      pattern[locationValue] = Frequencies[pitchValue]; 
    }
   
  }

  // Hitting the play button plays the current pattern, with the 
  // ocorrect tempo and length.
  if (digitalRead(playButton) == HIGH)
  {
    playPattern();
  }


  // The following if statement makes it so that the LCD only
  // refreshes if the values being displayed have been changed.
  if ((pitchValue != referencePitchValue) || (locationValue != referenceLocationValue) || (pattern[locationValue] != referenceLocationFrequencyValue))
  {

    
  //The LCD first displays the current Note that will
  // be heard if the soundButton is hit.  Then " - " is
  // displayed.  Then the current location in the pattern is
  // displayed.  Then " - " is displayed.  Then the current note
  // stored in that location is displayed.  On the next line of the
  // LCD, "T: " is displayed.  Then the current tempo is displayed.
  // Then " L: " is displayed.  Then the current pattern length is displayed.

    
    lcd.clear();
    lcd.print(Notes[pitchValue]);
    lcd.print(" - ");
    lcd.print(locationValue + 1);
    lcd.print(" - ");
    lcd.print(Notes[findFreqIndex(pattern[locationValue])]);

    //Move cursor to the second line of the LCD.
    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(map(delayTime, minDelayValue, maxDelayValue, 100, 1));
    lcd.print(" L: ");
    lcd.print(patternLength);
    //Move cursor back to the first line of the LCD. 
    lcd.setCursor(0, 0);
    

    // Reference values are now changed to what they currently are.
    // This way, the LCD will only update when one of the values 
    // is changed.
    referencePitchValue = pitchValue;
    referenceLocationValue = locationValue;
    referenceLocationFrequencyValue = pattern[locationValue];
  }

  // This short delay is included to alleviate some
  // stress on the Arduino.
  delay(50);
}


void playPattern(){

  // Traverses through the pattern array and plays the corresponding notes.
  // If there is a "0" in the array, no note is played, but the proper amount
  // of time is waited.
  
  for (int i = 0; i < patternLength; i++)
  { 
    lcd.clear();
    if (pattern[i] != 0)
    {
    tone(soundPiezo, pattern[i], delayTime);
    }

    // These three lines display th current location and note
    // being played in the pattern.  i + 1 is printed because 
    // this value would make more sense to the user.  For instance,
    // for a four note pattern, the function will play the frequencies
    // in indexes 0, 1, 2, and 3 of the pattern array, but to a user/musician,
    // it would make more sense to see 1, 2, 3, 4.
    lcd.print(i + 1);
    lcd.print(" - ");
    lcd.print(Notes[findFreqIndex(pattern[i])]);

    // This delay call is how the tempo functionality works.  The 
    // changeTempo function changes the value of delayTime, and in turn
    // changes how fast the notes will play.
    delay(delayTime);
  }
  
  displayCurrentValues();
}



void changeTempo(){
  lcd.clear();
  int tempoValue;

  // This referenceTempo variable is again used so that the
  // LCD only updates when a value is changed.    
  int referenceTempo = 0;

  // The user can scroll through all possible tempo values.
  // The playButton is used to exit the changeTempo mode.
  while (digitalRead(playButton) == LOW)
  {
    int tempoVoltage = analogRead(adjustPot);
    // Again, map and if statement written to avoid observed screen
    // flickering issue.
    tempoValue = map(tempoVoltage, 0, 1023, maxDelayValue, minDelayValue - 1);
    if (tempoValue == minDelayValue - 1)
    {
      tempoValue = minDelayValue;
    }

    // The LED should be at its brightest when tempoValue is at its lowest.
    int LEDBrightness = map(tempoValue, minDelayValue, maxDelayValue, 255, 0);
    analogWrite(tempoLED, LEDBrightness); 
    
    // Screen is only updated if tempoValue is changed. 
    if (tempoValue != referenceTempo)
    {

      // When displaying to tempo for the user, 1 will be the minimum tempo and 
      // 100 will be the maximum tempo.  This  inutivitvely makes more sense to the user compared
      // to displaying to them a delay time. 
      int displayTempoValue = map(tempoValue, minDelayValue, maxDelayValue, 100, 1);
      lcd.clear();
      lcd.print("Tempo: ");
      lcd.print(displayTempoValue);
      referenceTempo = tempoValue;  

      // playPattern uses delayTime, so changing delayTime will affect
      // how the pattern is played.  The result is the desired effect of 
      // changing the tempo.
      delayTime = tempoValue;
    }
  }

  // When leaving the function, occasionally only a blank
  // screen would be displayed, so these lines were included.
  lcd.clear();
  displayCurrentValues();
}


void changeLength(){
  lcd.clear();
  int lengthVoltage;
  int lengthValue;
  int referenceLengthValue = 0;

  // The user can scroll through all possible pattern length values.
  // The playButton is used to exit the changeLength mode.
  while (digitalRead(playButton) == LOW)
  {
    lengthVoltage = analogRead(adjustPot);
    lengthValue = map(lengthVoltage, 0, 1023, minPatternLength, maxPatternLength + 1);
    if (lengthValue == maxPatternLength + 1)
    {
      lengthValue = maxPatternLength;
    }
    if (lengthValue != referenceLengthValue)
    {
      lcd.clear();
      lcd.print("Length: ");
      lcd.print(lengthValue);
      referenceLengthValue = lengthValue; 
    } 
  } 

  // When the play button is hit, the pattern is played up 
  // to the index patternLength, so changing this index changes
  // the length of the array.
  patternLength = lengthValue;

  // When leaving the function, occasionally only a blank
  // screen would be displayed, so these lines were included.
  lcd.clear();
  displayCurrentValues();
}


// Sets all indexes of an array icluding and 
// after x equal to 0. 
void setToZeros(int x)
{
  for (int i = x; i < (((sizeof(pattern)) / (sizeof(int)))); i++)
  {
    pattern[i] = 0;
  }
}





// When leaving the changeTempo and changeLength functions,
// the screen would occasionally stay blank until a potentiometer
// was adjusted.  This function ensures that the this "blank screen"
// does not occur.  

void displayCurrentValues()
{
  int pitchVoltage = analogRead(pitchPot);
  int pitchValue = map(pitchVoltage, 0, 1023, 0, ((sizeof(Frequencies)) / (sizeof(int))));

  if (pitchValue == (sizeof(Frequencies)) / (sizeof(int)))
  {
      pitchValue = ((sizeof(Frequencies)) / (sizeof(int)) - 1);
  }


  int locationVoltage = analogRead(adjustPot);
  int locationValue = map(locationVoltage, 0, 1023, 0, patternLength);

  if (locationValue == patternLength)
  {
      locationValue = patternLength - 1;
  }

  //The LCD first displays the current Note that will
  // be heard if the soundButton is hit.  Then " - " is
  // displayed.  Then the current location in the pattern is
  // displayed.  Then " - " is displayed.  Then the current note
  // stored in that location is displayed.  On the next line of the
  // LCD, "T: " is displayed.  Then the current tempo is displayed.
  // Then " L: " is displayed.  Then the current pattern length is displayed.
    
   lcd.clear();
   lcd.print(Notes[pitchValue]);
   lcd.print(" - ");
   lcd.print(locationValue + 1);
   lcd.print(" - ");
   lcd.print(Notes[findFreqIndex(pattern[locationValue])]);
   //Move cursor to the second line of the LCD.
    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(map(delayTime, minDelayValue, maxDelayValue, 100, 1));
    lcd.print(" L: ");
    lcd.print(patternLength);
    //Move cursor back to the first line of the LCD. 
    lcd.setCursor(0, 0);

}



// This function returns the index of where a given
// frequency is in the Frequency array.  This function
// is useful in getting the name of a note given its frequency,
// since the Frequencies and Notes arrrays are parallel.   
int findFreqIndex(int x)
{
  boolean found = false;
  int ret = 0;
  for(int i = 0; i < ((sizeof(Frequencies)) / (sizeof(int))); i++)
  {
    if (Frequencies[i] == x)
    {
    found = true;
    ret = i;
    }
  }

  // If the variable found is never set to true, then
  // the given frequency is not in the Frequency array,
  // and -1 is returned.  
  if (found == true)
  {
  return ret;
  }
  else
  {
    // The program should only reach this else statement if an
    // invalid frequency is given to it (which should never happen),
    // but if this else block is triggered 1 is returned.  This 1 
    // corresponds to REST, which is a reasonable value to return if
    // an invalid frequnecy is given to the function.  (-1 could be 
    // returned, but that would surely cause the program to crash,
    // since the number being returned is supposed to be an index in
    // the Frequency array.)
    return 1;
  }
}


