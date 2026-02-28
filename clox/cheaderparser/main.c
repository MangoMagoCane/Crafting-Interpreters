#include <stdio.h>
#include <stdlib.h>

static char *
ReadFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof (char), fileSize, file);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0';
    fclose(file);

    return buffer;
}

static void
RunFile(const char *path)
{
    char *source = ReadFile(path);
}

int
main(int argc, char **argv)
{
    if (argc == 2)
    {
        RunFile(argv[1]);
    }
    else
    {
        exit(0); // note(val): Proper error code
    }

}
