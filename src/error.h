#if 1
#define Error(...) __debugbreak()
#else
void Error(char* string, short errorLevel)
{
   printf("Error: %s (error level %d)\n", string, errorLevel);
   exit(errorLevel);
}
#endif