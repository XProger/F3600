#define EUROPE 0
#define US     1
#define JAPAN  0
#define DEBUG  0

char SaveCardFile(char*, int, ConfigData*, short);
void GetHeader(char*);
char WriteFile(char*, char*, ConfigData*, short);
void LoadCardFiles(SelectionData*, char[][9], int);
char LoadCardData(char*, int, ConfigData*, SelectionData*);
long GetCardInfo(int);
void ClearEventHW();
long CardContentStatus();
void LoadCard(int);
long CheckHWCardEvents();
