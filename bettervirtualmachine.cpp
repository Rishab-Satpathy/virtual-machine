#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <exception>
using namespace std;

enum class opcode
{
    //basic calculator stuff
    PUSH,
    POP, 
    ADD,
    MUL, 
    DIV, 
    SUB,
    //jumping stuff
    POINTER, 
    JZ,
    JNZ,
    //comparission stuff
    EQ,
    LT,
    GT,
    HALT, 
};

enum VMSTATE
{
    RUNNING,
    PARSING,
    EXECUTING,
    ERROR,
    DEAD
};

VMSTATE state = RUNNING;

struct instruction
{
    opcode op;
    int operand;
    bool hasoperand;
};

//function to execute the program
void execution(const vector <instruction>& program)
{
    int ip = 0;
    vector <int> stack;
    while(state == EXECUTING && ip<program.size())
    {
        switch(program[ip].op)
        {
            case opcode::PUSH:
            stack.push_back(program[ip].operand);
            ip++;
            break;

            case opcode::ADD:
            {
            if (stack.size() < 2)
            {
                cout << "ADD not possible\n";
                state = ERROR;
                break;
            }

            int b = stack.back(); stack.pop_back();
            int a = stack.back(); stack.pop_back();
            stack.push_back(a + b);
            ip++;

            break;
            }

            case opcode::POINTER:
            {   
                int target = program[ip].operand;

                if (target < 0 || target >= program.size())
                {
                    cout << "Invalid jump at runtime\n";
                    state = ERROR;
                    break;
                }
                else
                {
                    ip = target;
                    break;
                }

            }


            case opcode::HALT:
            state = DEAD;
            return;

            case opcode::DIV:
            {
                if (stack.size() < 2)
            {
                cout << "DIV not possible\n";
                state = ERROR;
                break;
            }
            
            int b = stack.back();stack.pop_back();
            int a = stack.back();stack.pop_back();
            if(b!=0)
            {
            cout<<a/b<<endl;
            stack.push_back(a/b);
            ip++;
            break;
            }
            else
            {
                cout<<"division not possible";
                state=ERROR;
                break;
            }
            
            }

            case opcode::MUL:
            {
                if (stack.size() < 2)
            {
                cout << "MUL not possible\n";
                state = ERROR;
                break;
            }


            int a = stack.back();stack.pop_back();
            int b = stack.back();stack.pop_back();
            cout<<a*b<<endl;
            stack.push_back(a*b);
            ip++;
            break;
            }

            case opcode::SUB:
            {   
                if (stack.size() < 2)
            {
                cout << "SUB not possible\n";
                state = ERROR;
                break;
            }

            int a = stack.back();stack.pop_back();
            int b = stack.back();stack.pop_back();
            cout<<a-b<<endl;
            stack.push_back(a-b);
            ip++;
            break;
            }

            case opcode::POP:
            if (stack.empty())
            {
                cout << "POP not possible\n";
                state = ERROR;
            }
            else
            {
                cout<<stack.back();
                stack.pop_back();
                ip++;
            }
            break;
            
//conditional jumps here peek and pop the value from the stack thats present

            case opcode::JZ:
            {
                if (stack.empty())
                {
                    cout << "JZ needs a value\n";
                    state = ERROR;
                    return;
                }

                int condition = stack.back();
                stack.pop_back();

                if (condition == 0)
                {
                int target = program[ip].operand;
                if (target < 0 || target >= program.size())
                {
                cout << "Invalid jump\n";
                state = ERROR;
                return;
                }
                ip = target;
                }
                else
                ip++;
                break;
            }


            case opcode::JNZ:
            {
            if (stack.empty())
            {
                cout << "JNZ needs a value\n";
                state = ERROR;
                return;
            }

            int condition = stack.back();
            stack.pop_back();

            if (condition != 0)
            {
            int target = program[ip].operand;
            if (target < 0 || target >= program.size())
            {
                cout << "Invalid jump\n";
                state = ERROR;
                return;
            }

            ip = target;
            }
            else
            ip++;
            break;
            }

//execution for comparission commands
            case opcode::EQ:
            {
                if(stack.size()<2)
                {
                    cout<<"invalid stack size";
                    state = ERROR;
                    return;
                }
                int a = stack.back(); stack.pop_back();
                int b = stack.back(); stack.pop_back();
                stack.push_back(a == b ? 1 : 0);
                ip++;
                break;
            }

            case opcode::GT:
            {
                if(stack.size()<2)
                {
                    cout<<"invalid stack size";
                    state = ERROR;
                    return;
                }
                int a = stack.back(); stack.pop_back();
                int b = stack.back(); stack.pop_back();
                stack.push_back(a > b ? 1 : 0);
                ip++;
                break;
            }

            case opcode::LT:
             {
                if(stack.size()<2)
                {
                    cout<<"invalid stack size";
                    state = ERROR;
                    return;
                }
                int a = stack.back(); stack.pop_back();
                int b = stack.back(); stack.pop_back();
                stack.push_back(a < b ? 1 : 0);
                ip++;
                break;
            }

        }
    }

}

//function to handle the parsing and conversion of inputs to opcode
void parsing(const vector <string>& source, int k)
{   
    vector <instruction> program;
    for(int i = 0;i<source.size();i++)
    {
        if(source[i] == "PUSH")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                int value = stoi(source[++i]);
                program.push_back({opcode::PUSH, value, true});
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
                state=ERROR;
            }
            }
        }

        else if(source[i] == "POP")
        {
            program.push_back({opcode::POP, 0, false});
        }

        else if(source[i] == "ADD")
        {
            program.push_back({opcode::ADD,0,false});
        }

        else if(source[i] == "POINTER")
        {
            if(i+1>source.size())
            {
                cout<<"no operand found";
            }

            else
            {
            try
            {   
               int value = stoi(source[i + 1]);
                if (value < 0 || value >= k)
                {
                    cout << "Invalid jump\n";
                    state = ERROR;
                    return;
                }
                program.push_back({opcode::POINTER, value, true});
                i++;

            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
                state=ERROR;
            }
                 
            }
        }
        else if(source[i] == "MUL")
        {
            program.push_back({opcode::MUL,0,false});
        }
        else if(source[i] == "DIV")
        {
            program.push_back({opcode::DIV,0,false});
        }
        else if(source[i] == "SUB")
        {
            program.push_back({opcode::SUB,0,false});
        }
        else if(source[i] == "HALT")
        {
            program.push_back({opcode::HALT,0,false});
            state = EXECUTING;
        }

        else if (source[i] == "JZ")
        {
    if (i + 1 >= source.size())
    {
        cout << "JZ missing operand\n";
        state = ERROR;
        return;
    }
    try{
    int target = stoi(source[++i]);
    program.push_back({opcode::JZ, target, true});
    }
    catch(const std::exception& e)
    {
        cout << "Error: " << e.what() << endl;
    }
}
else if (source[i] == "JNZ")
{
    if (i + 1 >= source.size())
    {
        cout << "JNZ missing operand\n";
        state = ERROR;
        return;
    }

    try{
     int target = stoi(source[++i]);
    program.push_back({opcode::JNZ, target, true});
    }
    catch(const std::exception& e)
    {
        cout << "Error: " << e.what() << endl;
    }
}

//comparission parses
    else if(source[i] == "EQ")
    {
        program.push_back({opcode::EQ,0,false});
    }
    else if(source[i] == "GT")
    {
        program.push_back({opcode::GT,0,false});
    }
    else if(source[i] == "LT")
    {
        program.push_back({opcode::LT,0,false});
    }

    }
    if (state != EXECUTING)
    return;

    execution(program);

}

//function to handle inputs to the virtual machine
void input()
{
    vector<string> source;
    while(state == RUNNING)
    {   
        
        string command;
        cin>>command;
       if(command == "HALT")
       {
        source.push_back("HALT");
        state = PARSING;
       }
       else
       {
        source.push_back(command);
       }
    }
    parsing(source,source.size());
}

int main()
{   
    cout<<"The functions are"<<endl<<"PUSH"<<endl<<"POP"<<endl<<"POITNER"<<endl<<"MUL"<<endl<<"ADD"<<endl<<"DIV"<<endl<<"SUB"<<endl<<"HALT";
    input();
    if(state == DEAD)
    {
    cout<<"program over";
    }
    else if(state == ERROR)
    cout<<"program over with error";

    return 0;
}