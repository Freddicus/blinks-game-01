#include "reset.h"

void gameStateReset() {
  setValueSentOnAllFaces(Message::RESET_GAME);
  if ((millis() - gotResetSignalTime) > RESET_TIME) {
    initPlayVariables();
    setValueSentOnAllFaces(Message::QUIET);
  }
}