#include <iostream>
#include <windows.h>
#include <vector>
#include <sstream>
#include <conio.h>
#include <fstream>
#include <string>

using namespace std;


// Command function declarations
void listDirectory(const wstring& directory);
wstring stringToWString(const string& str);
wstring getCurrentDir();
string wstringToString(const wstring& wstr);

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

//execute python scripts D:

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
    std::string dir = (args.size() < 2) ? "." : args[1];
    listDirectory(stringToWString(dir));
    return;
}


    else if(args[0]=="help" && args.size()<2){
        
        string helptxt;
        ifstream helpFile("command_help.txt");
        while(getline(helpFile,helptxt)){
        cout<<helptxt<<endl;
        }
        helpFile.close();
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
    system("cls"); // Clear screen

    //current dir for the prefix
    wstring bfr=getCurrentDir();
    string currDir=wstringToString(bfr);


    string input;
    cout << "Type help -c for possible commands\nType netlib to list network utility functions\n";
    vector<string> shellPrefix;
    shellPrefix.push_back("\n$");
    shellPrefix.push_back(currDir);

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


    //debug section: to be made a comment or removed guys

    return 0;
}

// Lists files and directories in a directory
void listDirectory(const wstring& directory)
{
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

//helper functions

//string to w string
wstring stringToWString(const string& str){
    int size_needed=MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,nullptr,0);
    wstring wstr(size_needed,0);

    MultiByteToWideChar(CP_UTF8,0,str.c_str(),-1,&wstr[0],size_needed);

    if(!wstr.empty()&& wstr.back()==L'\0'){
        wstr.pop_back();
    }

    return wstr;
}

//wstring to string

string wstringToString(const wstring& wstr){
    int size_needed = WideCharToMultiByte(CP_UTF8,0,wstr.c_str(),-1,nullptr,0,NULL,NULL);
    string str(size_needed,0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);

    if(!str.empty() && str.back()=='\0'){
        str.pop_back();
    }

    return str;
}

//current directory
 wstring getCurrentDir(){
    DWORD length= GetCurrentDirectory(0,nullptr);
    wstring buffer(length,0);

    if(!buffer.empty() && buffer.back()==L'\0'){
        buffer.pop_back();
    }
    
    return buffer;}