#include <iostream>
#include <cctype>
#include <exception>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <SDL2/SDL.h>
using namespace std;

uint8_t R[16]; //16 registers 
uint8_t memory[4096];    //program data source
bool running = true;
uint16_t stack[16]; // stores the original program counter from where it jumps
uint8_t sp = 0;     // stack pointer
uint16_t I = 0;
SDL_AudioDeviceID audio_device;
bool audio_playing = false;


enum opcode : uint16_t
{
    MOV = 0x6000,
    ADD = 0x7000,
    JMP = 0x1000, //jumps to a given byte
    DRAW = 0XD000, //draws on the screen
    COPY = 0X8000, //copy function has both ADDXY for copy9ing and adding to a given register if its 0X8xy4 and only copying if its 0X8xy0
    CALL = 0X2000, //call a given byte
    RET = 0X00EE,
    LDI = 0XA000, //stores an address value to I
    ADDI = 0XF01E,//stores the value of register X with the value present in I already
    // FX33 = 0XF033, //decimal form of register stored in memory from index I to I+2
    // FX55 = 0XF055, //stores registers into memory from address I to final register
    // FX65 = 0XF065, //stores memory into registers from address I to final register
    // FX15 = 0XF015, //delay timer stores register X value
    // FX07 = 0XF007, //register stores delay timer value
    // FX18 = 0XF018, //sound timer stores register X value
    // EX9E = 0XE09E, //skip next if Vx pressed
    // EXA1 = 0XE0A1, //skip next if Vx not pressed
    // FX0A = 0XF00A, //wait for key press
    //FX29 = 0XFX29, //to use the fontset already provided
    // ADDXY = 0X8004,
    // SUBXY = 0X8005, //stores in R[x]
    // SUBYX = 0X8007, //stores in R[x]
    // SHR = 0X8006, //only for R[x]
    // SHL = 0X800E, //only for R[x]
    //HALT = 0XFFFF
};

enum VMSTATE
{
    RUNNING,
    PARSING
};
uint16_t mem_ptr = 0X200;

VMSTATE state = RUNNING;
//SDL stuff 
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
const int SCALE = 10;

//timer stuff
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;

//keyboard thingy
bool keys[16] = {false};
//graphics buffer
bool gfx[64 * 32];

//keypad input stuff
//SDL keyboard stuff
void process_sdl_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            running = false;

        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            bool pressed = (e.type == SDL_KEYDOWN);

            switch (e.key.keysym.sym)
            {
                case SDLK_1: keys[0x1] = pressed; break;
                case SDLK_2: keys[0x2] = pressed; break;
                case SDLK_3: keys[0x3] = pressed; break;
                case SDLK_4: keys[0xC] = pressed; break;

                case SDLK_q: keys[0x4] = pressed; break;
                case SDLK_w: keys[0x5] = pressed; break;
                case SDLK_e: keys[0x6] = pressed; break;
                case SDLK_r: keys[0xD] = pressed; break;

                case SDLK_a: keys[0x7] = pressed; break;
                case SDLK_s: keys[0x8] = pressed; break;
                case SDLK_d: keys[0x9] = pressed; break;
                case SDLK_f: keys[0xE] = pressed; break;

                case SDLK_z: keys[0xA] = pressed; break;
                case SDLK_x: keys[0x0] = pressed; break;
                case SDLK_c: keys[0xB] = pressed; break;
                case SDLK_v: keys[0xF] = pressed; break;
            }
        }
    }
}

//audio stuff
void audio_callback(void* userdata, Uint8* stream, int len)
{
    static int phase = 0;
    const int frequency = 440;      // Hz (classic CHIP-8 beep)
    const int sample_rate = 44100;
    const int amplitude = 3000;

    int16_t* buffer = (int16_t*)stream;
    int samples = len / sizeof(int16_t);

    if (!audio_playing)
    {
        memset(stream, 0, len);
        return;
    }

    for (int i = 0; i < samples; i++)
    {
        buffer[i] = (phase < sample_rate / (frequency * 2))
                        ? amplitude
                        : -amplitude;

        phase++;
        if (phase >= sample_rate / frequency)
            phase = 0;
    }
}


void tick_timers()
{
    if (delay_timer > 0)
        delay_timer--;

    if (sound_timer > 0)
        sound_timer--;
}
bool draw_flag = false;

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < 64 * 32; i++)
    {
        if (gfx[i])
        {
            SDL_Rect pixel = {
                (i % 64) * SCALE,
                (i / 64) * SCALE,
                SCALE,
                SCALE
            };
            SDL_RenderFillRect(renderer, &pixel);
        }
    }

    SDL_RenderPresent(renderer);
    draw_flag = false;
}

//parsing the function for the vm
void parsing(const vector<string>& source)
{

    for(int i=0;i<source.size();i++)
    {
        if(mem_ptr + 1 >= 4096)
        {
            cout << "Program too large\n";
            return;
        }

        
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
                uint16_t instr = 0X6000|(stoi(source[i+1]) << 8) |(stoi(source[i+2]));
                i +=2;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0XFF;
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
                
            }
            }
        } 
        
        else if(source[i] == "DRAW")
        {
         if(i+3>=source.size())
            {
                cout<<"no operator found";
            }
            else
            {
                try
                {
                    uint16_t instr =
                    0xD000 |(stoi(source[i+1]) << 8) |(stoi(source[i+2])<<4) | (stoi(source[i+3]));

                    i += 3;
                    memory[mem_ptr++] = instr >> 8;
                    memory[mem_ptr++] = instr & 0xFF;
                }       
                catch(const exception& e)
                {
                    cout << "Error: " << e.what() << endl;
                }
            }   
        }
        
        else if(source[i] == "FX29")
        {
         if(i+1>=source.size())
            {
                cout<<"no operator found";
            }
            else
            {
                try
                {
                    uint16_t instr =
                    0xF000 |(stoi(source[i+1]) << 8) |0x29;

                    i += 1;
                    memory[mem_ptr++] = instr >> 8;
                    memory[mem_ptr++] = instr & 0xFF;
                }       
                catch(const exception& e)
                {
                    cout << "Error: " << e.what() << endl;
                }
            }   
        }

        else if(source[i] == "ADD")
        {
             if(i+2>=source.size())
            {
                cout<<"no operator found";
            }
            else
            {
                try
                {
                    uint16_t instr =
                    0x7000 |(stoi(source[i+1]) << 8) |stoi(source[i+2]);

                    i += 2;
                    memory[mem_ptr++] = instr >> 8;
                    memory[mem_ptr++] = instr & 0xFF;
                }       
                catch(const exception& e)
                {
                    cout << "Error: " << e.what() << endl;
                }
            }
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
                uint16_t addr = (stoi(source[i + 1]) & 0x0FFF );

                uint16_t instr = 0x1000 | (addr);

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
                i += 1; 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "FX15")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t x = (stoi(source[i + 1]));

                uint16_t instr = 0xF000 | (x)<<8|0x15;

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
                i += 1; 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "FX18")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t x = (stoi(source[i + 1]));

                uint16_t instr = 0xF000 | (x)<<8|0x18;

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
                i += 1; 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "EX9E")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t x = (stoi(source[i + 1]));

                uint16_t instr = 0xE000 | (x)<<8|0x9E;

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
                i += 1; 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "EXA1")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t x = (stoi(source[i + 1]));

                uint16_t instr = 0xE000 | (x)<<8|0xA1;

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
                i += 1; 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "FX0A")
        {
            if(i+1>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t x = (stoi(source[i + 1]));

                uint16_t instr = 0xF000 | (x)<<8|0x0A;

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
                i += 1; 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "COPY")
        {
            if(i+2>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t rx = stoi(source[i + 1]);
                uint8_t ry = stoi(source[i + 2]);

                if(ry >= 16 || rx >= 16)
                {
                    cout << "Invalid register\n";
                    continue;
                }   

                uint16_t instr = 0x8000 | (rx << 8) | (ry << 4);

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;

                i += 2;
 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "ADDXY")
        {
            if(i+2>=source.size())
            {
                cout<<"no operator found";
            }


            else
            {
            try
            {
                uint8_t rx = stoi(source[i + 1]);
                uint8_t ry = stoi(source[i + 2]);

                if(rx >= 16 || ry >= 16)
                {
                    cout << "Invalid register\n";
                    continue;
                }   

                uint16_t instr = 0x8000 | (rx << 8) | (ry << 4)|0X0004;

                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;

                i += 2;
 
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
            }
        }

        else if(source[i] == "SUBXY")
        {
            if(i+2 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try
            {
                uint16_t instr = 0X8000|(stoi(source[i+1])<<8)|(stoi(source[i+2])<<4)|0X0005;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;

                i += 2;
            }
            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }

        }

        else if(source[i] == "SUBYX")
        {
            if(i+2 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try
            {
                uint16_t instr = 0X8000|(stoi(source[i+1])<<8)|(stoi(source[i+2])<<4)|0X0007;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;

                i += 2;
            }
            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }

        }

        else if(source[i] == "SHR")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try
            {
                uint16_t instr = 0X8000|(stoi(source[i+1])<<8)|0X0000|0X0006;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;

                i += 1;
            }
            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }

        }

        else if(source[i] == "SHL")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try
            {
                uint16_t instr = 0X8000|(stoi(source[i+1])<<8)|0X0000|0X000E;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;

                i += 1;
            }
            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }

        }

        else if(source[i] == "CALL")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try
            {
                uint16_t instr = 0X2000|(stoi(source[i+1]) & 0X0FFF);
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0XFF; 
                i++;
            }
            catch(const std::exception& e)
            {
                cout<<"Error: "<< e.what() <<endl;
            }
        }

        else if(source[i] == "LDI")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try
            {
                uint16_t instr = 0XA000|(stoi(source[i+1]) & 0X0FFF);
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0XFF; 
                i++;
            }
            catch(const std::exception& e)
            {
                cout<<"Error: "<< e.what() <<endl;
            }
        }

        else if(source[i] == "ADDI")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X001E;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

        else if(source[i] == "FX55")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X0055;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

         else if(source[i] == "FX65")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X0065;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

        else if(source[i] == "FX33")
        {
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X0033;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

        else if(source[i] == "FX15")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X0015;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

        else if(source[i] == "FX07")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X0007;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

        else if(source[i] == "FX18")
        {
            if(i+1 >= source.size())
            {
                cout<<"where the operand at?";
            }
            try{
            uint16_t instr = 0XF000|(stoi(source[i+1])<<8)|0X0018;
            memory[mem_ptr++] = instr >> 8;
            memory[mem_ptr++] = instr & 0xFF;
            i++;
            }
            catch(const std:: exception& e)
            {
                cout<<"Error: "<<e.what()<<endl;
            }

        }

        else if(source[i] == "RET")
        {
            
            try
            {
                uint16_t instr = 0X00EE;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
        }

        else if(source[i] == "HALT")
        {
            
            try
            {
                uint16_t instr = 0XFFFF;
                memory[mem_ptr++] = instr >> 8;
                memory[mem_ptr++] = instr & 0xFF;
            }

            catch (const std::exception& e)
            {
                cout << "Error: " << e.what() << endl;
            }
        }


    }
    execution();
}

uint16_t pc = 0;  // byte address (increments by 2)
//opcode stuff
void execute_one_instruction()
{
    
        uint16_t instr = memory[pc++]<<8 | memory[pc++];
        
        uint16_t opcode = instr & 0xF000;
        uint8_t  rx      = (instr & 0x0F00) >> 8;
        uint8_t  ry      = (instr & 0x00F0) >> 4;
        uint8_t  n      =  instr & 0x000F;
        uint8_t  imm     =  instr & 0x00FF;
        uint16_t addr   =  instr & 0x0FFF;


        if (instr == 0xFFFF) running = false;
        draw_flag = false;
        switch(opcode)
        {
            case 0X6000:
            {
                if(rx >= 16) { cout<<"Invalid register\n"; break; }
                R[rx] = imm;
                break;
            }

            case 0X7000:
            {
                if(rx >= 16) { cout << "Invalid register\n"; break; }
                R[rx] += imm;
                break;
            }

            case 0X1000: 
            {
                pc = addr;
                break;
            }

            case 0XF000:
            {
                switch(instr & 0X00FF)
                {
                case 0X00:
                {
                running = false;
                break;
                }

                case 0X1E:
                {
                    R[0xF] = (I + R[rx] > 0x0FFF);
                    I = (I + R[rx]) & 0x0FFF;
                    break;
                }

                case 0x33:
            {
                memory[I]     = R[rx] / 100;
                memory[I + 1] = (R[rx] / 10) % 10;
                memory[I + 2] = R[rx] % 10;
                break;
            }

            case 0x29:   // FX29 - LD F, Vx
            {
                I = 0x50 + (R[rx] * 5);
                break;
            }

            case 0x55:
            {
                for(int r = 0; r <= rx; r++)
                memory[I + r] = R[r];
                break;
            }

            case 0x65:
            {
                for(int r = 0; r <= rx; r++)
                R[r] = memory[I + r];
                break;
            }

            case 0x07:
                R[rx] = delay_timer;
                break;

            case 0x15:
                delay_timer = R[rx];
                break;

            case 0x18:
                sound_timer = R[rx];
                break;

            case 0x0A:
                {
                    bool key_pressed = false;
                    for (int k = 0; k < 16; k++)
                {
                    if (keys[k])
                    {
                    R[rx] = k;
                    key_pressed = true;
                    break;
                    }
                }

                if (!key_pressed)
                pc -= 2; // repeat instruction
                }
                break;

                }
                break;
            
            }

            case 0X8000:   // opcode == 0x8000
            {
                switch(n)
                {       
                    case 0x0:
                    {
                    R[rx] = R[ry];
                    break;
                    }

                    case 0x4:
                    {
                    uint16_t sum = R[rx] + R[ry];
                    R[0xF] = (sum > 255);
                    R[rx] = sum & 0xFF;
                    }
                    break;

                    default:
                    cout << "Invalid 8XYn instruction\n";
                    running = false;
                    break;

                    case 0X5:
                    {
                    R[0xF] = ((R[rx] >= R[ry])?1:0);
                    uint16_t sum = R[rx] - R[ry];
                    R[rx] = sum & 0xFF;
                    break;
                    }

                    case 0X7:
                    {
                    R[0xF] = ((R[ry] >= R[rx])?1:0);
                    uint16_t sum = R[ry] - R[rx];
                    R[rx] = sum & 0xFF;
                    break;
                    }   

                    case 0X6:
                    {
                    R[0xF] = R[rx] & 1;
                    R[rx] >>= 1;
                    break;
                    }

                    case 0XE:
                    {
                    R[0xF] = (R[rx] & 0x80) >> 7;
                    R[rx] = (R[rx] << 1) & 0xFF;
                    break;
                    }
                }
                break;
            }

            case 0X2000:
            {   
                if(sp >= 16)
                {
                    cout<<"Stack Overflow"<<endl;
                    running = false;
                    break;
                }
                stack[sp++] = pc;
                pc = addr;
                break;
            }

            case 0XA000:
            {
                I = addr;
                break;
            }

            case 0xE000:
            switch (instr & 0x00FF)
            {
                case 0x9E:
                    if (keys[R[rx]])
                    pc += 2;
                    break;

                case 0xA1:
                    if (!keys[R[rx]])
                    pc += 2;
                    break;
            break;
            }
            

            case 0xD000:
            {
                uint8_t x = R[rx] % 64;
                uint8_t y = R[ry] % 32;
                R[0xF] = 0;

                for (int row = 0; row < n; row++)
                {
                    uint8_t sprite = memory[I + row];
                    for (int col = 0; col < 8; col++)
                    {
                        if (sprite & (0x80 >> col))
                        {
                            int px = (x + col) % 64;
                            int py = (y + row) % 32;
                            int index = py * 64 + px;

                            if (gfx[index])
                            R[0xF] = 1;

                            gfx[index] ^= 1;
                        }
                    }
                }

                draw_flag = true;
            }
            break;

            default:
            {
                if(instr == 0x00EE) // RET
                {
                    if(sp == 0) { cout << "Stack underflow\n"; running = false; break; }
                    pc = stack[--sp]; // pop PC
                }
                else
                {
                    cout << "Unknown opcode " << hex << instr << endl;
                    running = false;
                }
                break;
            }

        }

}

void execution()
{
    pc = 0X200;
    running = true;
    auto last_timer_tick = std::chrono::high_resolution_clock::now();

    while(running)
    {
        if(pc + 1 >= 4096)
        {
            cout << "PC out of bounds\n";
            break;
        }
        
        // if (_kbhit())   // Windows only; OK for now
        // {
        //     char key = _getch();
        //     handle_key(key, true);
        // }
        process_sdl_events();

        for (int i = 0; i < 10; i++)
        {
            execute_one_instruction();
        }

        if(draw_flag == true)
        render();
//keyboard stuff
// memset(keys, false, sizeof(keys));

// --- timer logic ---
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - last_timer_tick
                   );

    bool did_tick = false;

while (elapsed.count() >= 16)
{
    tick_timers();
    last_timer_tick += std::chrono::milliseconds(16);
    elapsed -= std::chrono::milliseconds(16);
    did_tick = true;
}

if (sound_timer > 0)
    audio_playing = true;
else
    audio_playing = false;

    }
}

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
    parsing(source);
}

void load_fontset()
{
    uint8_t fontset[80] = {
        0xF0,0x90,0x90,0x90,0xF0, // 0
        0x20,0x60,0x20,0x20,0x70, // 1
        0xF0,0x10,0xF0,0x80,0xF0, // 2
        0xF0,0x10,0xF0,0x10,0xF0, // 3
        0x90,0x90,0xF0,0x10,0x10, // 4
        0xF0,0x80,0xF0,0x10,0xF0, // 5
        0xF0,0x80,0xF0,0x90,0xF0, // 6
        0xF0,0x10,0x20,0x40,0x40, // 7
        0xF0,0x90,0xF0,0x90,0xF0, // 8
        0xF0,0x90,0xF0,0x10,0xF0, // 9
        0xF0,0x90,0xF0,0x90,0x90, // A
        0xE0,0x90,0xE0,0x90,0xE0, // B
        0xF0,0x80,0x80,0x80,0xF0, // C
        0xE0,0x90,0x90,0x90,0xE0, // D
        0xF0,0x80,0xF0,0x80,0xF0, // E
        0xF0,0x80,0xF0,0x80,0x80  // F
    };

    for (int i = 0; i < 80; i++)
        memory[0x50 + i] = fontset[i];
}

bool init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        cout << "SDL_Init failed: " << SDL_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 320,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        cout << "Window creation failed: " << SDL_GetError() << endl;
        return false;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer)
    {
        cout << "Renderer creation failed: " << SDL_GetError() << endl;
        return false;
    }

    SDL_AudioSpec want{};
want.freq = 44100;
want.format = AUDIO_S16SYS;
want.channels = 1;
want.samples = 512;
want.callback = audio_callback;

audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);

if (!audio_device)
{
    cout << "SDL audio failed: " << SDL_GetError() << endl;
    return false;
}

SDL_PauseAudioDevice(audio_device, 0); // start audio

return true;
}

int main()
{

     if (!init_sdl())
        return 1;

     load_fontset();

     cout<<"The functions are"<<endl<<"MOV"<<endl<<"ADD"<<endl<<"JMP"<<endl<<"HALT";
     input();

     SDL_DestroyRenderer(renderer);
     SDL_DestroyWindow(window);
     SDL_Quit();

     if(running == false)
     {
        cout<<endl<<"the end";
     }

     return 0;
}