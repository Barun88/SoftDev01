#include <iostream>
#include <windows.h>
#include<vector>
#include<sstream>
#include<conio.h>

/*to do for tommorow
1.add an auto fill option 
2.add default windows command 
3.make a gui
*/
using namespace std;

vector<string> parseIn(const string &input)
{
    vector<string> tokens;
    istringstream stream(input);
    string token;

    while(stream>>token)
    {
        tokens.push_back(token);
    }

    return tokens;
}

void executeCommand(const vector<string>& args,vector<string> &prefix)
{
    if(args.empty())return;
    
    if(args[0]=="cd")
    {
        if(args.size()<2)
        {
            cerr<<"Error: Missing Directory\n";
        }
        else{
            if(!SetCurrentDirectory(args[1].c_str()))
            {
                cerr<<"Error: Could not change directory\n";
            }
            else{
                prefix.push_back(args[1]);
                prefix.push_back("/");
            }
        }
        return;
    }
    else if(args[0]=="cls")
    {
        system("cls");
        return;
    }

    string commandLine; //converting string to char * for create process
    for(const string&arg:args)
    {
        commandLine += arg+" ";//concatanation ["this","is","a","command"]->"this is a command";
    }

    STARTUPINFO si={sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi;
    char cmd[1024];
    strcpy_s(cmd,commandLine.c_str());

    if(CreateProcess(NULL,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)){
        WaitForSingleObject(pi.hProcess,INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else{
        cerr<<"Error:Command not found.\n";
    }
}

void commandList(const string & input){
    vector<string> validCommands={"cd","cls","ping","dir","echo","exit"};
    cout<<"\nAuto Complete Suggestions: ";
    for(const auto& cmd:validCommands )
    {
        if(cmd.find(input)==0)
        {
            cout<<cmd<<" ";
        }
    }
    cout<<endl<<"$"<<input;
}

string getUserInput() {
    string input;
    char ch;
    while (true) {
        ch = _getch(); // Get character without Enter key
        if (ch == '\r') break;  // Stop on Enter
        if (ch == '\b') {  // Backspace handling
            if (!input.empty()) {
                cout << "\b \b";  // Move cursor back, erase character
                input.pop_back(); // Remove last character from string
            }
            continue;
        }
        if (ch == '\t') { 
            commandList(input); // Show auto-complete suggestions
            continue;
        }

        cout << ch;
        input += ch;
    }

    cout << endl;
    return input;
}

int main(void)
{
    //cli-outlook commands
    system("cls");

    string input;
    cout<<"Type help -c  for possible command\n";
    vector<string> shellPrefix;
    shellPrefix.push_back("$");



    //main loop
    while(true)
    {
    for(auto itr:shellPrefix)
    {
        cout<<itr;
    }
    input=getUserInput();

    if(input =="ez"||input=="exit")
    {
        break;
    }
    vector<string> parsed=parseIn(input);
    executeCommand(parsed,shellPrefix);
    }

    return 0;

}
