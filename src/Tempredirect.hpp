#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

struct TempRedirect
{
    std::string path;
    int fd = -1;
    FILE *file = nullptr;

    bool Create(const char *prefix);

    void Close();

    ~TempRedirect();
};