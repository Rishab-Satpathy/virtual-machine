#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <cctype>
using namespace std;

//global variables
int ip=0;
int position=1;

enum VMState { RUNNING, DEAD };
VMState state = RUNNING;

vector <int> stack;

void functions(const string function,const vector <string>& cmds)
{   
    int numbering=-1;
    if(function == "push")
    {
        numbering = 0;
    }
    else if(function == "pop")
    {
        if(stack.empty())
        {
            cout << "not enough integers in stack\n";
            return;
        }
        else
        numbering = 1;
    }
    else if(function == "add")
    {
      
        numbering = 2;
    }
    else if(function == "halt")
    {
     
        numbering = -1;
    }
    else if(function == "pointer")
    {
        
        numbering = 3;
    }

    switch(numbering)
    {
        case 0:
        {   
            try
            {
            int value = stoi(cmds[position]);
            stack.push_back(value);
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }

            break;
        }
        
        case 1:
        {   
            if(stack.size()<1)
            {
                cout<<"not enough integers in stack";
                break;
            }
            cout<<"the stack element is"<<endl;
            cout<<stack.back() << endl; // Output the top element
            stack.pop_back();
            break;
        }

        case 2:
        {
            if(stack.size()<2)
            {
                cout<<"error:not enough integers";
                return;
            }
            int a = stack.back();
            stack.pop_back();
            int b = stack.back();
            stack.pop_back();
            cout<<a+b<<endl;
            stack.push_back(a+b);
            break;
        }
        case 3:
        {
            int value;
            try
            {
                value = stoi(cmds[position]);
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }

            if(value < cmds.size() && value >= 0)
            {
                bool digit = false;
                if (cmds[value].empty()) digit = false;

                 int start = 0;
                if (cmds[value][0] == '+' || cmds[value][0] == '-')
                start = 1;

                if (start == cmds[value].size()) digit = false;

                for (int i = start; i < cmds[value].size(); i++)
                {
                    if (!isdigit(cmds[value][i]))
                    {
                    digit = false;
                    break;
                    }
                    else 
                    digit = true;
                }

                if(digit)
                {
                    cout<<"not a function";
                    state = DEAD;
                    break;
                }
                else 
                {
                    ip = value;
                    break;
                }
            }
            else
            {
              cout<<"out of bounds"<<endl;
              break;
            }
        }
        case -1:
        {
            cout<<"the end";
            state = DEAD;
            return;
        }
    }
}
int main()
{
    vector <string> commands;
    bool halt = true;
    cout<<"the commands of this virtual machine are:"<<endl<<"push"<<endl<<"pop"<<endl<<"halt"<<endl;
    while(halt==true)
    {
        //input of commands
        string command;
        cin>>command;

        //conversion to lowercase
        for(int j=0;j<command.length();j++)
        {
            if(command[j]>=65 && command[j]<=90)
            {
                command[j]+=32;
            }
        }

        //inserting commands to vector
        commands.push_back(command);

        //command to break the loop
        if(command == "halt")
        {
            halt=false;
        }
    }

    //function for executing commands

  while (ip < commands.size() && state == RUNNING)
{
    if ((commands[ip] == "push" || commands[ip] == "pointer") && ip + 1 < commands.size())
    {
        position = ip + 1;
        functions(commands[ip], commands);
        ip += 2;
    }
    else
    {
        functions(commands[ip], commands);
        ip++;
    }
}

return 0;
}