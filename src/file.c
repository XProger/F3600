/* (C) Psygnosis 1994. By Jason Carl Denton
*/

#include "game.h"

long FileLength(const char* name)
{
/*TODO 3DO*/
#ifdef __3DO__
    return 0;
#else
    FILE* fd;
    long length;

    /* Open file */

    fd = fopen(name, "rb");
    if (fd == NULL)
    {
        return 0;
    }

    /* Seek to end of file to get length */

    fseek(fd, 0, SEEK_END);
    length = ftell(fd);
    rewind(fd);

    /* Close file */

    fclose(fd);

    /* Return file length */

    return (length);
#endif
}

long LoadFile(const char* name, char* buffer)
{
/*TODO 3DO*/
#ifdef __3DO__
    return 0;
#else
    FILE* fd;
    long length;

    /* Open file */

    fd = fopen(name, "rb");
    if (fd == NULL)
    {
        ASSERT(0);
        return (0);
    }

    /* Seek to end of file to get length */

    fseek(fd, 0, SEEK_END);
    length = ftell(fd);
    /* Seek back to the start */
    rewind(fd);

    /* Read file into buffer */

    if (fread(buffer, 1, length, fd) != length)
    {
        ASSERT(0);
        fclose(fd);
        return (0);
    }

    /* Close file */

    fclose(fd);

    /* Return file length */

    return (length);
#endif
}
