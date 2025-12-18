#include <iostream>
#include <cctype>
#include <exception>
#include <string>
#include <vector>
#include <cstdint>
using namespace std;

uint8_t R[4]; //4 opcode functions
uint16_t pc=0;  //number of instructions
uint8_t memory[256];    //program data source
bool running = true;

enum opcode : uint8_t
{
    MOV = 0x01,
    ADD = 0x02,
    JMP = 0x03,
    HALT = 0xFF
};

enum VMSTATE
{
    RUNNING,
    PARSING
};
uint16_t mem_ptr = 0;

VMSTATE state = RUNNING;
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

void parsing(const vector<string>& source, int size)
{

    for(int i=0;i<source.size();i++)
    {
        
        if(source[i] == "MOV")
        {
            if(i+2>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                memory[mem_ptr++] = MOV;
                memory[mem_ptr++] = stoi(source[i + 1]); // rX
                memory[mem_ptr++] = stoi(source[i + 2]); // imm
                i += 2;

            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
                state=ERROR;
            }
            }
        }

        else if(source[i] == "ADD")
        {
             if(i+2>=source.size())
            {
                cout<<"no operator found";
            }
            memory[mem_ptr++] = ADD;
            memory[mem_ptr++] = stoi(source[i + 1]); // rX
            memory[mem_ptr++] = stoi(source[i + 2]); // rY
            i += 2;

        }
        else if(source[i] == "JMP")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                memory[mem_ptr++] = JMP;
                memory[mem_ptr++] = stoi(source[i + 1]);
                i += 1;  
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
                state=ERROR;
            }
            }
        }
        else if(source[i] == "HALT")
        {
            memory[mem_ptr++] = HALT;
        }

    }
    void execution();
}

void execution()
{
    while(running)
    {
        uint8_t opcode = memory[pc++];

        switch(opcode)
        {
            case MOV:
            {
                uint8_t rx = memory[pc++];
                uint8_t imm = memory[pc++];
                R[rx] = imm;
                cout<<R[rx]<<endl;
                break;
            }

            case ADD:
            {
                uint8_t rx = memory[pc++];
                uint8_t ry = memory[pc++];
                R[rx] += R[ry];
                cout<<R[rx]<<endl;
                break;
            }

            case JMP:
            {
                uint8_t addr = memory[pc++];
                pc = addr;
                break;
            }

            case HALT:
            {
                running = false;
                break;
            }

            default:
            {
                cout<<"Wrong opcode";
                running = false;
                break;
            }

        }

    }
}

int main()
{
     cout<<"The functions are"<<endl<<"MOV"<<endl<<"ADD"<<endl<<"JMP"<<endl<<"HALT";
     input();
     if(running == false)
     {
        cout<<endl<<"the end";
     }

     return 0;
}