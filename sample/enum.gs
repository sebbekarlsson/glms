typedef enum {
  EMOTION_HAPPY,
  EMOTION_ANGRY,
  EMOTION_SAD
} EmotionState;


EmotionState j = EmotionState.EMOTION_ANGRY;


switch (j) {
  case EmotionState.EMOTION_HAPPY: print("Happy"); break;
  case EmotionState.EMOTION_ANGRY: print("Angry"); break;
  case EmotionState.EMOTION_SAD: print("Sad"); break;
}