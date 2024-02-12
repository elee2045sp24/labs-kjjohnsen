#include <M5StickCPlus2.h>

void setup() {
  pinMode(4, OUTPUT); // Set HOLD pin 04 as output
  digitalWrite(4, HIGH);  //IMPORTANT, Set HOLD pin to high to maintain power supply or M5StickCP2 will turn off if not USB connected
  auto cfg = M5.config();
  StickCP2.begin(cfg);
}

//these global variables are effectively the things we remember between iterations of the loop
int state = 1;
long pressTime;
long releaseTime;

//our main loop is constantly checking the state and input/time to determine what to do next
void loop() {
  StickCP2.update();
  
  if(state == 1){  //always checking to see if the button was pressed 
    if(StickCP2.BtnA.wasPressed()){
      StickCP2.Speaker.tone(2000); 
      pressTime = millis();
      state = 2;
    }
  }
  else if(state == 2){ //always checking to see if the button was released
    if(StickCP2.BtnA.wasReleased()){
      StickCP2.Speaker.stop();
      releaseTime = millis();
      long duration = releaseTime-pressTime;
      //this does a . if short, and a - if long
      if(duration < 300){
        Serial.print(".");
      }else{
        Serial.print("-");
      }
      state = 3;
    }
  }
  else if(state == 3){ //most complicated state, because it has the most directions
    long silenceDuration = millis() - releaseTime;
    if(silenceDuration > 3000){ //very long silence means go back to state 1 (new word)
      Serial.print("\n");
      state = 1;
    }else if(StickCP2.BtnA.wasPressed()){ //pressing a button in time means that we skip straight to state 1.  
      StickCP2.Speaker.tone(2000); //in theory, we could make this a function, because it's repetitive, but that seemed wastful. 
      pressTime = millis();
      if(silenceDuration < 1000){ //if this was a short release, then it's part of the letter (no space)
        state = 2;
      }else{ //a duration between 1000 and 3000 is a new letter.
        Serial.print(" ");
        state = 2;
      }
    }
  }
}
