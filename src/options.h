#define FADE_INACTIVE 0
#define FADE_ACTIVE   1
#define FADE_COMPLETE 2
#define FADE_OUT      3

void InGameReset(int choice, ConfigData*, int*);
void InGameConfirm(char* confirm, int choice, short xpad, char* lConfirm);
const char* GetMusicTrackName(int musicTrack);
