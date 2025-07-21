#include <iostream>
#include <windows.h>
#include <vector>
#include <sstream>
#include <conio.h>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

// --- Function Declarations ---
bool listDirectory(const wstring& directory);
wstring stringToWString(const string& str);
string wstringToString(const wstring& wstr);
wstring getCurrentDir();
vector<string> parseIn(const string& input);
void executeCommand(const vector<string>& args, vector<string>& prefix);
string getUserInput();
void commandList(const string& input);
bool fileExists(const string& filename);
bool directoryExists(const string& dirname);

// --- Check if file exists ---
bool fileExists(const string& filename) {
    DWORD dwAttrib = GetFileAttributesA(filename.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// --- Check if directory exists ---
bool directoryExists(const string& dirname) {
    DWORD dwAttrib = GetFileAttributesA(dirname.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

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
    if (args.empty()) {
        return;
    }

    // Handle "util" command with proper error checking
    if (args.size() >= 2 && args[0] == "util") {
        wchar_t exePath[MAX_PATH];
        DWORD result = GetModuleFileNameW(NULL, exePath, MAX_PATH);
        
        if (result == 0) {
            cerr << "Error: Could not get executable path. Error code: " << GetLastError() << endl;
            return;
        }
        
        if (result >= MAX_PATH) {
            cerr << "Error: Executable path too long" << endl;
            return;
        }

        wstring exePathStr(exePath);
        string exeDir = wstringToString(exePathStr);
        
        if (exeDir.empty()) {
            cerr << "Error: Failed to convert executable path" << endl;
            return;
        }

        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash == string::npos) {
            cerr << "Error: Invalid executable path format" << endl;
            return;
        }
        
        exeDir = exeDir.substr(0, lastSlash);
        string scriptsPath = exeDir + "/scripts/";
        
        if (!directoryExists(scriptsPath)) {
            cerr << "Error: Scripts directory not found: " << scriptsPath << endl;
            return;
        }

        string scriptFile = scriptsPath + args[1] + ".py";
        if (!fileExists(scriptFile)) {
            cerr << "Error: Script file not found: " << scriptFile << endl;
            return;
        }

        vector<string> newArgs = {"python", scriptFile};
        
        // Add additional arguments if provided
        for (size_t i = 2; i < args.size(); ++i) {
            newArgs.push_back(args[i]);
        }
        
        executeCommand(newArgs, prefix);
        return;
    }

    // Handle "cd" command
    if (args[0] == "cd") {
        if (args.size() < 2) {
            cerr << "Error: Missing directory argument" << endl;
            return;
        }
        
        if (!directoryExists(args[1])) {
            cerr << "Error: Directory does not exist: " << args[1] << endl;
            return;
        }
        
        if (!SetCurrentDirectoryA(args[1].c_str())) {
            cerr << "Error: Could not change directory to '" << args[1] 
                 << "'. Error code: " << GetLastError() << endl;
            return;
        }
        
        wstring wideDir = getCurrentDir();
        if (wideDir.empty()) {
            cerr << "Warning: Could not get current directory after change" << endl;
            return;
        }
        
        string currDir = wstringToString(wideDir);
        if (!currDir.empty()) {
            prefix = {"\n$", currDir, ">"};
        }
        return;
    }
    
    // Handle "cls" command
    if (args[0] == "cls") {
        if (system("cls") != 0) {
            cerr << "Warning: Failed to clear screen" << endl;
        }
        return;
    }
    
    // Handle "ld" (list directory) command
    if (args[0] == "ld") {
        string dir = (args.size() < 2) ? "." : args[1];
        
        if (!directoryExists(dir)) {
            cerr << "Error: Directory does not exist: " << dir << endl;
            return;
        }
        
        if (!listDirectory(stringToWString(dir))) {
            cerr << "Error: Failed to list directory contents" << endl;
        }
        return;
    }
    
    // Handle "cat" command
    if (args[0] == "cat") {
        if (args.size() < 2) {
            cerr << "Error: Missing file name for 'cat'" << endl;
            return;
        }
        
        if (!fileExists(args[1])) {
            cerr << "Error: File does not exist: " << args[1] << endl;
            return;
        }
        
        ifstream file(args[1]);
        if (!file.is_open()) {
            cerr << "Error: Could not open file '" << args[1] << "'" << endl;
            return;
        }
        
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        
        if (file.bad()) {
            cerr << "Error: Problem reading file '" << args[1] << "'" << endl;
        }
        
        file.close();
        return;
    }
    
    // Handle "help" command
    if (args[0] == "help" && args.size() < 2) {
        const string helpFile = "command_help.txt";
        
        if (!fileExists(helpFile)) {
            cout << "Available commands:" << endl;
            cout << "  cd <directory>     - Change directory" << endl;
            cout << "  cls                - Clear screen" << endl;
            cout << "  ld [directory]     - List directory contents" << endl;
            cout << "  cat <file>         - Display file contents" << endl;
            cout << "  mkdir <directory>  - Create directory" << endl;
            cout << "  rm <file>          - Delete file" << endl;
            cout << "  util <script> [args] - Run Python script" << endl;
            cout << "  help               - Show this help" << endl;
            cout << "  exit/ez            - Exit shell" << endl;
            return;
        }
        
        ifstream file(helpFile);
        if (!file.is_open()) {
            cerr << "Error: Could not open help file" << endl;
            return;
        }
        
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        
        if (file.bad()) {
            cerr << "Error: Problem reading help file" << endl;
        }
        
        file.close();
        return;
    }

    // Handle "mkdir" command
    if (args[0] == "mkdir") {
        if (args.size() < 2) {
            cerr << "Error: Missing directory name" << endl;
            return;
        }
        
        if (directoryExists(args[1])) {
            cerr << "Error: Directory already exists: " << args[1] << endl;
            return;
        }
        
        if (!CreateDirectoryA(args[1].c_str(), NULL)) {
            DWORD error = GetLastError();
            cerr << "Error: Could not create directory '" << args[1] 
                 << "'. Error code: " << error << endl;
        } else {
            cout << "Directory created successfully: " << args[1] << endl;
        }
        return;
    }

    // Handle "rm" command
    if (args[0] == "rm") {
        if (args.size() < 2) {
            cerr << "Error: Missing file name" << endl;
            return;
        }
        
        if (!fileExists(args[1])) {
            cerr << "Error: File does not exist: " << args[1] << endl;
            return;
        }
        
        if (!DeleteFileA(args[1].c_str())) {
            DWORD error = GetLastError();
            cerr << "Error: Could not delete file '" << args[1] 
                 << "'. Error code: " << error << endl;
        } else {
            cout << "File deleted successfully: " << args[1] << endl;
        }
        return;
    }

    // Execute as external process
    string commandLine;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) commandLine += " ";
        
        // Quote arguments that contain spaces
        if (args[i].find(' ') != string::npos) {
            commandLine += "\"" + args[i] + "\"";
        } else {
            commandLine += args[i];
        }
    }

    if (commandLine.length() >= 1024) {
        cerr << "Error: Command line too long (max 1023 characters)" << endl;
        return;
    }

    char cmd[1024];
    strcpy_s(cmd, sizeof(cmd), commandLine.c_str());
    
    STARTUPINFOA si = {};
    si.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION pi = {};

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        DWORD error = GetLastError();
        cerr << "Error: Could not execute command '" << args[0] 
             << "'. Error code: " << error << endl;
        return;
    }

    // Wait for process completion
    DWORD waitResult = WaitForSingleObject(pi.hProcess, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        cerr << "Warning: Process wait failed" << endl;
    }

    // Get exit code
    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
        if (exitCode != 0) {
            cout << "Process exited with code: " << exitCode << endl;
        }
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// --- Command auto-complete ---
void commandList(const string& input) {
    vector<string> validCommands = {"cd", "cls", "ping", "echo", "exit", "ld", "cat", "mkdir", "rm", "util", "help"};
    vector<string> matches;
    
    for (const auto& cmd : validCommands) {
        if (cmd.find(input) == 0) {
            matches.push_back(cmd);
        }
    }
    
    if (!matches.empty()) {
        cout << "\nAuto Complete Suggestions: ";
        for (const auto& match : matches) {
            cout << match << " ";
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
        
        if (ch == '\r' || ch == '\n') {
            break; // Enter key
        }
        
        if (ch == '\b' || ch == 127) { // Backspace
            if (!input.empty()) {
                cout << "\b \b";
                input.pop_back();
            }
            continue;
        }
        
        if (ch == '\t') { // Tab key
            commandList(input);
            continue;
        }
        
        if (ch == 27) { // Escape key
            // Clear current input
            for (size_t i = 0; i < input.length(); ++i) {
                cout << "\b \b";
            }
            input.clear();
            continue;
        }
        
        // Only accept printable characters
        if (ch >= 32 && ch <= 126) {
            cout << ch;
            input += ch;
        }
    }
    
    cout << endl;
    return input;
}

// --- Main function ---
int main() {
    // Initialize console
    if (system("cls") != 0) {
        cerr << "Warning: Could not clear screen" << endl;
    }
    
    // Set locale for proper Unicode handling
    try {
        locale::global(locale(""));
        wcout.imbue(locale());
    } catch (const exception& e) {
        cerr << "Warning: Could not set locale: " << e.what() << endl;
    }

    // Get initial directory
    wstring wideDir = getCurrentDir();
    if (wideDir.empty()) {
        cerr << "Error: Could not get current directory" << endl;
        return 1;
    }
    
    string currDir = wstringToString(wideDir);
    if (currDir.empty()) {
        cerr << "Error: Could not convert current directory path" << endl;
        return 1;
    }

    cout << "Custom Shell v1.0" << endl;
    cout << "Type 'help' for available commands" << endl;
    cout << "Type 'exit' or 'ez' to quit" << endl;
    
    vector<string> shellPrefix = {"\n$", currDir, ">"};

    while (true) {
        try {
            // Display prompt
            for (const auto& part : shellPrefix) {
                cout << part;
            }
            cout << " ";

            // Get user input
            string input = getUserInput();
            
            // Check for exit commands
            if (input == "ez" || input == "exit") {
                cout << "Goodbye!" << endl;
                break;
            }
            
            // Skip empty input
            if (input.empty()) {
                continue;
            }

            // Parse and execute command
            vector<string> parsed = parseIn(input);
            executeCommand(parsed, shellPrefix);
            
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        } catch (...) {
            cerr << "Unknown error occurred" << endl;
        }
    }
    
    return 0;
}

// --- Directory listing with error checking ---
bool listDirectory(const wstring& directory) {
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind;

    wstring searchPath = directory;
    if (!searchPath.empty() && searchPath.back() != L'\\' && searchPath.back() != L'/') {
        searchPath += L"\\";
    }
    searchPath += L"*";

    hFind = FindFirstFileW(searchPath.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        wcerr << L"Error: Could not open directory '" << directory 
              << L"'. Error code: " << error << endl;
        return false;
    }

    do {
        // Skip current and parent directory entries
        if (wcscmp(findFileData.cFileName, L".") == 0 || 
            wcscmp(findFileData.cFileName, L"..") == 0) {
            continue;
        }
        
        wcout << findFileData.cFileName;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wcout << L" [DIR]";
        } else {
            // Show file size
            LARGE_INTEGER fileSize;
            fileSize.LowPart = findFileData.nFileSizeLow;
            fileSize.HighPart = findFileData.nFileSizeHigh;
            wcout << L" (" << fileSize.QuadPart << L" bytes)";
        }
        wcout << endl;
        
    } while (FindNextFileW(hFind, &findFileData) != 0);

    DWORD error = GetLastError();
    FindClose(hFind);
    
    if (error != ERROR_NO_MORE_FILES) {
        wcerr << L"Warning: Directory listing may be incomplete. Error code: " 
              << error << endl;
        return false;
    }
    
    return true;
}

// --- String to WString with error checking ---
wstring stringToWString(const string& str) {
    if (str.empty()) {
        return wstring();
    }
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (size_needed <= 0) {
        wcerr << L"Error: Could not convert string to wide string" << endl;
        return wstring();
    }
    
    wstring wstr(size_needed, 0);
    int result = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
    if (result <= 0) {
        wcerr << L"Error: String conversion failed" << endl;
        return wstring();
    }
    
    // Remove null terminator if present
    if (!wstr.empty() && wstr.back() == L'\0') {
        wstr.pop_back();
    }
    
    return wstr;
}

// --- WString to String with error checking ---
string wstringToString(const wstring& wstr) {
    if (wstr.empty()) {
        return string();
    }
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, NULL, NULL);
    if (size_needed <= 0) {
        cerr << "Error: Could not convert wide string to string" << endl;
        return string();
    }
    
    string str(size_needed, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);
    if (result <= 0) {
        cerr << "Error: Wide string conversion failed" << endl;
        return string();
    }
    
    // Remove null terminator if present
    if (!str.empty() && str.back() == '\0') {
        str.pop_back();
    }
    
    return str;
}

// --- Get current working directory with error checking ---
wstring getCurrentDir() {
    DWORD length = GetCurrentDirectoryW(0, nullptr);
    if (length == 0) {
        wcerr << L"Error: Failed to get current directory length. Error code: " 
              << GetLastError() << endl;
        return wstring();
    }

    wstring buffer(length, 0);
    DWORD result = GetCurrentDirectoryW(length, &buffer[0]);
    if (result == 0) {
        wcerr << L"Error: Failed to get current directory. Error code: " 
              << GetLastError() << endl;
        return wstring();
    }
    
    if (result >= length) {
        wcerr << L"Error: Current directory path truncated" << endl;
        return wstring();
    }

    // Remove null terminator if present
    if (!buffer.empty() && buffer.back() == L'\0') {
        buffer.pop_back();
    }
    
    return buffer;
}