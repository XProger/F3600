char SaveCardFile(char*, int, ConfigData*, short);
void GetHeader(char*);
void LoadCardFiles(SelectionData*, char[][9], int);
char LoadCardData(char*, int, ConfigData*, SelectionData*);
long GetCardInfo(int);
long CardContentStatus(void);
void LoadCard(int);
long CheckHWCardEvents(void);
