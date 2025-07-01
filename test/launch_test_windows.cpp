#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

std::wstring ConvertToWString(const std::string &input)
{
    if (input.empty())
        return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

int main()
{
    HANDLE stdout_read = nullptr;
    HANDLE stdout_write = nullptr;

    SECURITY_ATTRIBUTES saAttr = {};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&stdout_read, &stdout_write, &saAttr, 0))
    {
        std::cerr << "Failed to create stdout pipe\n";
        return 1;
    }

    // Make read handle non-inheritable (only parent reads from it)
    SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOEX startupinfoex = {};
    startupinfoex.StartupInfo.cb = sizeof(STARTUPINFOEX);
    startupinfoex.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupinfoex.StartupInfo.hStdOutput = stdout_write;
    startupinfoex.StartupInfo.hStdError = stdout_write;
    startupinfoex.StartupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE); // optional

    SIZE_T attr_list_size = 0;
    InitializeProcThreadAttributeList(NULL, 1, 0, &attr_list_size);
    startupinfoex.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)malloc(attr_list_size);
    InitializeProcThreadAttributeList(startupinfoex.lpAttributeList, 1, 0, &attr_list_size);

    HANDLE handle_list[2] = { stdout_write, GetStdHandle(STD_INPUT_HANDLE) };
    UpdateProcThreadAttribute(
        startupinfoex.lpAttributeList, 0,
        PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
        handle_list,
        sizeof(handle_list),
        nullptr,
        nullptr);

    PROCESS_INFORMATION pi = {};
    std::wstring wexecutable = ConvertToWString("Z:\\Projects\\lldb-frontend\\build-windows\\Debug\\lldb-frontend-test.exe");
    std::wstring wcommandLine = ConvertToWString("\"Z:\\Projects\\lldb-frontend\\build-windows\\Debug\\lldb-frontend-test.exe\"");
    std::wstring wworkingDirectory = ConvertToWString("Z:\\Projects\\lldb-frontend\\build-windows\\Debug");
    WCHAR *pwcommandLine = &wcommandLine[0];

    BOOL result = CreateProcessW(
        wexecutable.c_str(),
        pwcommandLine,
        NULL,
        NULL,
        TRUE, // inherit handles
        EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT,
        NULL,
        wworkingDirectory.c_str(),
        (LPSTARTUPINFOW)&startupinfoex,
        &pi);

    if (!result)
    {
        std::cerr << "CreateProcessW failed with error: " << GetLastError() << std::endl;
        return 2;
    }

    CloseHandle(stdout_write); // Close write-end in parent

    // Read child stdout
    DWORD read = 0;
    CHAR buffer[4096];
    while (ReadFile(stdout_read, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0)
    {
        buffer[read] = '\0';
        std::cout << buffer;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(stdout_read);

    DeleteProcThreadAttributeList(startupinfoex.lpAttributeList);
    free(startupinfoex.lpAttributeList);

    return 0;
}
