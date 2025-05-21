#include <iostream>
#include <windows.h>
#include <vector>
#include <sstream>
#include <conio.h>
#include <fstream>
#include <string>

using namespace std;

// --- Function Declarations ---
void listDirectory(const wstring& directory);
wstring stringToWString(const string& str);
string wstringToString(const wstring& wstr);
wstring getCurrentDir();
vector<string> parseIn(const string& input);
void executeCommand(const vector<string>& args, vector<string>& prefix);
string getUserInput();
void commandList(const string& input);

// --- Parse input string into tokens ---
vector<string> parseIn(const string& input) {
    vector<string> tokens;
    istringstream stream(input);
    string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// --- Execute shell-like commands ---
void executeCommand(const vector<string>& args, vector<string>& prefix) {
    if (args.empty()) return;

    if (args.size() > 1 && args[0] == "util") {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    string exeDir = wstringToString(wstring(exePath));
    exeDir = exeDir.substr(0, exeDir.find_last_of("\\/"));

    string scriptsPath = exeDir + "/scripts/";
    vector<string> newArgs = { "python", scriptsPath + args[1] + ".py" };
    
    executeCommand(newArgs, prefix);
    return;
}

    if (args[0] == "cd") {
        if (args.size() < 2) {
            cerr << "Error: Missing directory\n";
        } else {
            if (!SetCurrentDirectoryA(args[1].c_str())) {
                cerr << "Error: Could not change directory\n";
            } else {
                wstring wideDir = getCurrentDir();
                string currDir = wstringToString(wideDir);
                prefix = { "\n$", currDir, ">" };
            }
        }
        return;
    }
    else if (args[0] == "cls") {
        system("cls");
        return;
    }
    else if (args[0] == "ld") {
        string dir = (args.size() < 2) ? "." : args[1];
        listDirectory(stringToWString(dir));
        return;
    }
    else if (args[0] == "cat") {
        if (args.size() < 2) {
            cerr << "Error: Missing file name for 'cat'\n";
            return;
        }
        ifstream file(args[1]);
        if (!file) {
            cerr << "Error: Could not open file\n";
            return;
        }
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
        return;
    }
    else if (args[0] == "help" && args.size() < 2) {
        ifstream helpFile("command_help.txt");
        string line;
        while (getline(helpFile, line)) {
            cout << line << endl;
        }
        helpFile.close();
        return;
    }

    else if (args[0] == "mkdir") {
    if (args.size() < 2) {
        cerr << "Error: Missing directory name\n";
    } else {
        if (!CreateDirectoryA(args[1].c_str(), NULL)) {
            cerr << "Error: Could not create directory\n";
        }
    }
    return;
    }

    else if (args[0] == "rm") {
    if (args.size() < 2) {
        cerr << "Error: Missing file name\n";
    } else {
        if (!DeleteFileA(args[1].c_str())) {
            cerr << "Error: Could not delete file\n";
        }
    }
    return;
    }



    // Execute as external process
    string commandLine;
    for (const string& arg : args) {
        commandLine += arg + " ";
    }

    char cmd[1024];
    if (commandLine.length() >= sizeof(cmd)) {
        cerr << "Error: Command too long.\n";
        return;
    }

    strcpy_s(cmd, commandLine.c_str());
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        cerr << "Error: Command not found.\n";
    }
}

// --- Command auto-complete ---
void commandList(const string& input) {
    vector<string> validCommands = { "cd", "cls", "ping", "echo", "exit", "ld", "cat","mkdir","rm" };
    cout << "\nAuto Complete Suggestions: ";
    for (const auto& cmd : validCommands) {
        if (cmd.find(input) == 0) {
            cout << cmd << " ";
        }
    }
    cout << endl << "$" << input;
}

// --- Get user input with tab-autocomplete ---
string getUserInput() {
    string input;
    char ch;
    while (true) {
        ch = _getch();
        if (ch == '\r') break; // Enter
        if (ch == '\b') {
            if (!input.empty()) {
                cout << "\b \b";
                input.pop_back();
            }
            continue;
        }
        if (ch == '\t') {
            commandList(input);
            continue;
        }
        cout << ch;
        input += ch;
    }
    cout << endl;
    return input;
}

// --- Main function ---
int main() {
    system("cls");
    locale::global(locale(""));
    wcout.imbue(locale());

    wstring wideDir = getCurrentDir();
    string currDir = wstringToString(wideDir);

    cout << "Type help -c for possible commands\nType netlib to list network utility functions\n";
    vector<string> shellPrefix = { "\n$", currDir, ">" };

    while (true) {
        for (const auto& part : shellPrefix) {
            cout << part;
        }
        cout << " ";

        string input = getUserInput();
        if (input == "ez" || input == "exit") break;

        vector<string> parsed = parseIn(input);
        executeCommand(parsed, shellPrefix);
    }
    return 0;
}

// --- Directory listing ---
void listDirectory(const wstring& directory) {
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind;

    wstring searchPath = directory;
    if (directory.back() != L'\\') searchPath += L"\\";
    searchPath += L"*";

    hFind = FindFirstFileW(searchPath.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        wcerr << L"Error: Could not open directory. Error Code: " << GetLastError() << endl;
        return;
    }

    do {
        wcout << findFileData.cFileName;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            wcout << L" [DIR]";
        wcout << endl;
    } while (FindNextFileW(hFind, &findFileData) != 0);

    FindClose(hFind);
}

// --- String to WString ---
wstring stringToWString(const string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    if (!wstr.empty() && wstr.back() == L'\0') {
        wstr.pop_back();
    }
    return wstr;
}

// --- WString to String ---
string wstringToString(const wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, NULL, NULL);
    string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);
    if (!str.empty() && str.back() == '\0') {
        str.pop_back();
    }
    return str;
}

// --- Get current working directory ---
wstring getCurrentDir() {
    DWORD length = GetCurrentDirectoryW(0, nullptr);
    if (length == 0) {
        wcerr << L"[getCurrentDir] Failed to get length. Error: " << GetLastError() << endl;
        return L"";
    }

    wstring buffer(length, 0);
    DWORD result = GetCurrentDirectoryW(length, &buffer[0]);
    if (result == 0 || result >= length) {
        wcerr << L"[getCurrentDir] Failed to get path. Error: " << GetLastError() << endl;
        return L"";
    }

    if (!buffer.empty() && buffer.back() == L'\0') {
        buffer.pop_back();
    }
    return buffer;
}
