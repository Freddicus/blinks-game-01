#include "setup.h"

bool gotSetupMsg;

void initSetupVariables() {
  gotSetupMsg = false;
}

void gameStateSetup() {
  // button double clicked - need to tell neighbors to start
  if (buttonDoubleClicked()) {
    // we're in setup mode (currently at startup only)
    // indicate to the user to detach all blinks on all blinks
    setValueSentOnAllFaces(Message::PLEASE_DETACH);

    // originator got the setup message
    gotSetupMsg = true;
    return;
  }

  FOREACH_FACE(f) {
    byte faceValue = getLastValueReceivedOnFace(f);
    bool faceValueExpired = isValueReceivedOnFaceExpired(f);

    // receiving the please detach message
    if (faceValue == Message::PLEASE_DETACH && !faceValueExpired) {
      // pass it along to everyone!
      setValueSentOnAllFaces(Message::PLEASE_DETACH);
    } else if (faceValue == Message::PLEASE_DETACH && faceValueExpired) {
      // no longer actively receiving detach message - note and indicate
      gotSetupMsg = true;
    }
  }  // for each face

  // if i'm alone, then i'm ready to play
  if (gotSetupMsg && isAlone()) {
    gameState = GameState::PLAYING;
    // don't send anything
    setValueSentOnAllFaces(Message::QUIET);
  }
}