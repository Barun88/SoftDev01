#include <iostream>
#include <windows.h>
#include <vector>
#include <sstream>
#include <conio.h>

using namespace std;

// Command function declarations
void listDirectory(const string& directory);

// Input parser
vector<string> parseIn(const string &input)
{
    vector<string> tokens;
    istringstream stream(input);
    string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

// Execute shell-like commands
void executeCommand(const vector<string>& args, vector<string>& prefix)
{
    if (args.empty()) return;

    if (args[0] == "cd") {
        if (args.size() < 2) {
            cerr << "Error: Missing directory\n";
        } else {
            if (!SetCurrentDirectory(args[1].c_str())) {
                cerr << "Error: Could not change directory\n";
            } else {
                prefix.push_back(args[1]);
                prefix.push_back("/");
            }
        }
        return;
    }
    else if (args[0] == "cls") {
        system("cls");
        return;
    }
    else if (args[0] == "ld") {
        if (args.size() < 2) {
            cerr << "Error: Missing directory for 'ld'\n";
            return;
        }
        listDirectory(args[1]);
        return;
    }

    // Concatenate args into a single command line string
    string commandLine;
    for (const string& arg : args) {
        commandLine += arg + " ";
    }

    // Check command length
    char cmd[1024];
    if (commandLine.length() >= sizeof(cmd)) {
        cerr << "Error: Command too long.\n";
        return;
    }

    strcpy_s(cmd, commandLine.c_str());

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        cerr << "Error: Command not found.\n";
    }
}

// Autocomplete suggestions for commands
void commandList(const string& input)
{
    vector<string> validCommands = { "cd", "cls", "ping", "dir", "echo", "exit", "ld" };
    cout << "\nAuto Complete Suggestions: ";
    for (const auto& cmd : validCommands) {
        if (cmd.find(input) == 0) {
            cout << cmd << " ";
        }
    }
    cout << endl << "$" << input;
}

// Get input from user with autocomplete support
string getUserInput()
{
    string input;
    char ch;
    while (true) {
        ch = _getch();
        if (ch == '\r') break; // Enter key
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

// Main function
int main()
{
    system("cls"); // Clear screen once at start

    string input;
    cout << "Type help -c for possible commands\n";
    vector<string> shellPrefix;
    shellPrefix.push_back("$");

    // Command loop
    while (true) {
        for (const auto& itr : shellPrefix) {
            cout << itr;
        }
        cout << " "; // Better formatting

        input = getUserInput();

        if (input == "ez" || input == "exit") {
            break;
        }

        vector<string> parsed = parseIn(input);
        executeCommand(parsed, shellPrefix);
    }

    return 0;
}

// Lists files and directories in a directory
void listDirectory(const string& directory)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    string searchPath = directory + "\\*";
    hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "Error: Could not open directory." << endl;
        return;
    }

    do {
        cout << findFileData.cFileName;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            cout << " [DIR]";
        cout << endl;
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}
