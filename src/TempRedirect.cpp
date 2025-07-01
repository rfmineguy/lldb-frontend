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

    path = tmp_filename;
    fd = _open(tmp_filename, _O_RDWR | _O_BINARY);
    if (fd == -1)
        return false;

    file = _fdopen(fd, "w+");
    return file != nullptr;
#else
    char tmp_filename[] = "/tmp/lldb_io_XXXXXX";
    fd = mkstemp(tmp_filename);
    if (fd == -1)
        return false;

    path = tmp_filename;
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