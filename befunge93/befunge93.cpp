#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <array>



struct RandomBuffer
{
    uint32_t idx;
    uint32_t length;
    uint8_t buffer[0x100];
};

struct Map
{
    int x;
    int y;
    uint8_t* buffer; 
};

enum {
    EAST = 0,
    WEST,
    SOUTH,
    NORTH,
};


struct RandomBuffer random_buffer;
struct Map* map = NULL;


void errQuit(const char* err_msg){
    fprintf(stderr,"%s\n",err_msg);
    _exit(-1);
}

void initRandomBuffer(){
    int fd = open("/dev/urandom",O_RDONLY);
    if(fd < 0){
        errQuit("open /dev/urandom failed!");
    }
    random_buffer.length = 0x100;
    read(fd,random_buffer.buffer,random_buffer.length);
    random_buffer.idx = 0;
    close(fd);
}

uint8_t getRandom(){
    if(random_buffer.idx >= random_buffer.length){
        initRandomBuffer();
    }
    return random_buffer.buffer[random_buffer.idx++];
}

void initMap(int x,int y){
    if(map != NULL){
        free(map->buffer);
        free(map);
    }
    map = (struct Map*)malloc(x*y);
    if(map == NULL){
        errQuit("malloc failed!");
    }
    map->x = x;
    map->y = y;
    map->buffer = (uint8_t*)malloc(x*y+0x10);
    if(map->buffer == NULL){
        errQuit("malloc failed!");
    }
}

int read_n(uint8_t* buffer,int length){
    int n = 0;
    int cur = 0;
    while(cur < length){
        n = read(0,buffer+cur,1);
        if(n <= 0)
            return -1;
        cur++;
    }
    return cur;
}


int get_int(){
    uint8_t buffer[0x10] = {0};
    read(0,buffer,0xf);
    return atoi((const char*)buffer);
}

uint32_t get_uint(){
    return (uint32_t)get_int();
}

void banner(){ 
    printf("%s\n",
"######   #####  ####### #######  #####    ###    #####   #####  \n"
"#     # #     #    #    #       #     #  #   #  #     # #     # \n"
"#     # #          #    #             # #     #       #       # \n"
"######  #          #    #####    #####  #     #  #####   #####  \n"
"#   #   #          #    #       #       #     # #       #       \n"
"#    #  #     #    #    #       #        #   #  #       #       \n"
"#     #  #####     #    #       #######   ###   ####### ####### \n"
    );
}

void init(){
    setbuf(stdin,NULL);
    setbuf(stdout,NULL);
    setbuf(stderr,NULL);
    initRandomBuffer();
}

void showCode(int code_length){
    int y = map->y;
    uint8_t* buffer = map->buffer;

    for(int i = 0;i < code_length;i++){
        if(i % y == 0)
            putchar('\n');
        printf("%c",buffer[i]);
    }
}

// https://github.com/qwertybomb/befunge-93/blob/main/b93.cc
void interpreter(uint8_t* code){
    std::vector<uint32_t> stack;
    int x = 0;
    int y = 0;
    uint8_t* code_ptr = map->buffer;
    int rows = map->x;
    int cols = map->y;
    int dirs[4][2] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0},
    };
    int direction = EAST;       // default is '>'

    auto push = [&](uint32_t value) -> void { stack.push_back(value); };
    auto pop = [&]() -> uint32_t
    {
        if (stack.empty())
        {
            return 0;
        } 
        else
        {
            uint32_t temp = stack.back();
            stack.pop_back();
            return temp;
        }
    };
    auto move = [&]() -> void {
        x = (x + dirs[direction][0]) % rows; 
        y = (y + dirs[direction][1]) % cols;
    };


    while(1){
        uint8_t code = code_ptr[x*cols+y];
        switch (code){
        case '+':
            {
                push(pop()+pop());
            }
            break;
        case '-':
            {
                uint32_t a = pop();
                uint32_t b = pop();
                push(b-a);
            }
            break;
        case '*':
            {
                push(pop()*pop());
            }
            break;
        case '/':
            {
                uint32_t a = pop();
                uint32_t b = pop();
                if(a == 0)
                    push(0);
                else
                    push(b / a);
            }
            break;
        case '%':
            {
                uint32_t a = pop();
                uint32_t b = pop();
                if(a == 0)
                    push(0);
                else
                    push(b % a);
            }
            break;
        case '!':
            {
                if (stack.empty())
                {
                    /* 0 == 0 is true */
                    push(1);
                } 
                else
                {
                    stack.back() = stack.back() == 0;
                }
            }   
            break;
         case '`':
            {
                uint32_t a = pop();
                uint32_t b = pop();
                push(b > a);
            } 
            break;
        case '^':
            {
                direction = NORTH;
            } 
            break;
        case 'v':
            {
                direction = SOUTH;
            } 
            break;
        case '>':
            {
                direction = EAST;
            } 
            break;
        case '<':
            {
               direction = WEST;
            } 
            break;
        case '_':
            {
                direction = pop() == 0 ? EAST : WEST;
            } 
            break;
        case '|':
            {
                direction = pop() == 0 ? SOUTH : NORTH;
            } 
            break;
        case '"':
            {
                move();
                /* while the current ch is not a quote push its ascii value */
                for (;;)
                {   
                    uint8_t ch = code_ptr[x*cols+y];
                    if (ch != '"')
                    {
                        push(ch);
                        move();
                    } 
                    else
                    {
                        break;
                    }
                }
            } 
            break;
        case ':':
            {
                push(stack.empty() ? 0 : stack.back());
            } 
            break;
        case '\\':
            {
                if(stack.size() == 0){
                    ;
                }else if(stack.size() == 1){
                    push(0);
                }else{
                    uint32_t a = pop();
                    uint32_t b = pop();
                    push(a);
                    push(b);
                }

            }
            break;
        case '$':
            {
                pop();
            } 
            break;
        case '.':
            {
                uint32_t value = pop();
                printf("%d", value);
            } 
            break;
        case ',':
            {
                uint8_t value = (uint8_t)pop();
                write(1,&value,1);
            }
            break;
        case '#':
            {
                move();
            } 
            break;
        case 'g':
            {

                int y_ = (int)pop();
                int x_ = (int)pop();

                // vuln here
                push((x_ < rows && y_ < cols) ? code_ptr[x_ * cols + y_] : 0);
            } 
            break;
        case 'p':
            {
                int y_ = (int)pop();
                int x_ = (int)pop();
                uint8_t value = (uint8_t)pop();

                // vule here
                if(x_ < rows && y_ < cols){
                    code_ptr[x_ * cols + y_] = value; 
                }
            } 
            break;
        case '&':
            {
                uint32_t value = get_uint();
                push(value);
            } 
            break;
        case '~':
            {
                uint8_t value;
                read(0,&value,1);
                push(value);
            } 
            break;
        // exit
        case '@': 
            return;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                push(code - '0');
            } 
            break;
        case '?':
            {
                direction = getRandom() % 4;
            } 
            break;
        default:
            break;
        }
        move();
    }
}

int main(){
    int x,y;
    int code_length;
    int n;
    banner();
    init();
    printf("Welecome to RCTF2022!\n");
    printf("This is just a simple Befunge93 interpreter!\n");

    // initial code area
    printf("input x:\n");
    x = get_int();
    printf("input y:\n");
    y = get_int();
    if(x <= 0 || y <= 0)
        errQuit("invalid x or y\n");
    if(x > 0x1000 || y > 0x1000)
        errQuit("invalid x or y\n");
    initMap(x,y);
    // read user code
    printf("input your code length:\n");
    code_length = get_int();
    if(code_length <= 0 || code_length > x*y)
        errQuit("invalid code_length\n");
    printf("input your code:\n");
    n = read_n(map->buffer,code_length);
    if(n <= 0)
        errQuit("read error!\n");
    showCode(code_length);
    // now run code
    interpreter(map->buffer);
    if(map != NULL){
        free(map->buffer);
        free(map);
    }    

    return 0;
}