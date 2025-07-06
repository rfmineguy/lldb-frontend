#include <fmt/format.h>

bool TempRedirect::Create(const char *prefix)
{
    if (file)
        Close();
#ifdef _WIN32
    char tmp_path[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, tmp_path))
        return false;

    char tmp_filename[MAX_PATH];
    if (!GetTempFileNameA(tmp_path, prefix, 0, tmp_filename))
        return false;

    path = std::filesystem::path(tmp_filename);
    fd = _open(tmp_filename, _O_RDWR | _O_BINARY);
    if (fd == -1)
        return false;

    file = _fdopen(fd, "w+");
    return file != nullptr;
#else
    std::string fmt = fmt::format("/tmp/lldb_io_{}_XXXXXXX", prefix);
    fd = mkstemp(fmt.data());
    if (fd == -1)
        return false;

    path = std::filesystem::path(fmt.c_str());
    file = fdopen(fd, "w+");
    return file != nullptr;
#endif
}

void TempRedirect::Close()
{
    if (file)
    {
        fclose(file);
        file = nullptr;
        std::filesystem::remove(path);
    }
    else if (fd != -1)
    {
#ifdef _WIN32
        _close(fd);
#else
        close(fd);
#endif
        fd = -1;
    }
}

TempRedirect::~TempRedirect()
{
    Close();
}