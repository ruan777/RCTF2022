#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

struct RandomBuffer
{
    uint32_t idx;
    uint32_t length;
    uint8_t buffer[0x200];
};

class NoteBook {
public:
    uint8_t second_,minutes_,hour_,day_,month_;
    uint16_t year_;
    bool is_encrypt_;
    char* content_;

    NoteBook(uint8_t second = 0,uint8_t minutes = 0,uint8_t hour = 0,uint8_t day = 1,
    uint8_t month = 1,uint16_t year = 1970,const char* content = nullptr,bool is_encrypt = false){
            second_ = second;
            minutes_ = minutes;
            hour_ = hour;
            day_ = day;
            month_ = month;
            year_ = year;
            is_encrypt_ = is_encrypt;
            content_ = nullptr;
            if(content != nullptr){
                content_ = new char[0x300];
                int content_len = strlen(content);
                memcpy(content_,content,content_len >= 0x2f0 ? 0x2f0 : content_len);
            }
                
            // printf("call NoteBook()\n");
    }

    // NoteBook& operator=(const NoteBook& another) {
    //     this->second_ = another.second_;
    //     this->minutes_ = another.minutes_;
    //     this->hour_ = another.hour_;
    //     this->day_ = another.day_;
    //     this->month_ = another.month_;
    //     this->year_ = another.year_;
    //     this->is_encrypt_ = another.is_encrypt_;
    //     this->content_ = another.content_;
    //     return *this;
    // }  

    // NoteBook(const NoteBook& another) {
    //     this->second_ = another.second_;
    //     this->minutes_ = another.minutes_;
    //     this->hour_ = another.hour_;
    //     this->day_ = another.day_;
    //     this->month_ = another.month_;
    //     this->year_ = another.year_;
    //     this->is_encrypt_ = another.is_encrypt_;
    //     this->content_ = another.content_;
    //     printf("call NoteBook(const NoteBook& another)\n");
    // }

    ~NoteBook() {
        if(content_ != nullptr && check_valid())
            delete [] content_;  
        // printf("call ~NoteBook()\n");
    }

    void updateNote(uint8_t second = 0,uint8_t minutes = 0,uint8_t hour = 0,uint8_t day = 1,
    uint8_t month = 1,uint16_t year = 1970,const char* content = nullptr){
            second_ = second;
            minutes_ = minutes;
            hour_ = hour;
            day_ = day;
            month_ = month;
            year_ = year;
            if(content != nullptr){
                if(content_ == nullptr)
                    content_ = new char[0x300];
                int content_len = strlen(content);
                memset(content_,0,0x300);
                memcpy(content_,"    ",4);
                memcpy(content_ + 4,content,content_len >= 0x2f0 ? 0x2f0 : content_len);
            }
                
            // printf("call updateNote()\n");
    }

    void updateNote(const char* content){
        if(content != nullptr){
            if(content_ == nullptr)
                content_ = new char[0x300];
            int content_len = strlen(content);
            memset(content_,0,0x300);
            memcpy(content_,"    ",4);
            memcpy(content_ + 4,content,content_len >= 0x2f0 ? 0x2f0 : content_len);
        }    
        // printf("call updateNote()\n");
    }

private:
    bool check_valid() {
        if(!memcmp(content_,"    ",4))
            return true;
        return false;
    }
};

enum operation {
    ADD = 1,
    UPDATE,
    SHOW,
    DELETE,
    ENCRYPT,
    DECRYPT
};

struct enc{
    uint32_t offset;
    uint32_t length;
    char* original;
};


RandomBuffer random_buffer;
std::map<std::string,operation> m;
std::map<uint64_t,std::vector<enc*>> enc_info;
std::vector<NoteBook> notes;
std::set<uint64_t> note_hash;

void errQuit(const char* err_msg){
    fprintf(stderr,"%s\n",err_msg);
    _exit(-1);
}

void initRandomBuffer(){
    int fd = open("/dev/urandom",O_RDONLY);
    if(fd < 0){
        errQuit("open /dev/urandom failed!");
    }
    random_buffer.length = 0x200;
    read(fd,random_buffer.buffer,random_buffer.length);
    random_buffer.idx = 0;
    close(fd);
}

uint8_t getRandom(){
    if(random_buffer.idx >= random_buffer.length){
        random_buffer.idx = 0;
    }
    return random_buffer.buffer[random_buffer.idx++];
}

void init() {
    setbuf(stdin,NULL);
    setbuf(stdout,NULL);
    setbuf(stderr,NULL);
    initRandomBuffer();
    m["add"] = ADD;
    m["update"] = UPDATE;
    m["show"] = SHOW;
    m["delete"] = DELETE;
    m["encrypt"] = ENCRYPT;
    m["decrypt"] = DECRYPT;
    notes.reserve(0x20);
}

int read_line(char* buf,int max_num) {
    int i = 0;
    while(i < max_num) {
        read(0,buf + i,1);
        if(buf[i] == '\n')
            break;
        i++;
    }
    buf[i] = 0;
    return i;
}

// from https://stackoverflow.com/questions/26328793/how-to-split-string-with-delimiter-using-c
void split(const std::string& s, std::vector<std::string>& tokens, char delim = '#') {
    tokens.clear();
    size_t lastPos = s.find_first_not_of(delim, 0);
    size_t pos = s.find(delim, lastPos);
    while (lastPos != std::string::npos) {
        tokens.emplace_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delim, pos);
        pos = s.find(delim, lastPos);
    }
}

void add(std::vector<std::string>& cmds) {
    if(cmds.size() != 8){
        fprintf(stderr,"%s\n","add format: add#year#month#day#hour#minutes#second#content");
        return;
    }

    uint32_t year = std::stoi(cmds[1]);
    uint32_t month = std::stoi(cmds[2]);
    uint32_t day = std::stoi(cmds[3]);
    uint32_t hour = std::stoi(cmds[4]);
    uint32_t minutes = std::stoi(cmds[5]);
    uint32_t second = std::stoi(cmds[6]);
    const char* content = cmds[7].c_str();

    if(second > 59 || minutes > 59 || hour > 23 || month > 12 || day > 31 || year > 2022) {
        fprintf(stderr,"%s\n","invalid date");
        return;
    }

    if(notes.size() >= 0x20){
        fprintf(stderr,"%s\n","no space avilable,please delete some notes");
        return;
    }

    uint64_t hash = (uint64_t)year << 40 |
                (uint64_t)month << 32 |
                day << 24 |
                hour << 16 |
                minutes << 8 |
                second;
    
    if(note_hash.find(hash) != note_hash.end()){
        fprintf(stderr,"%s\n","invalid date");
        return;
    }

    note_hash.insert(hash);

    notes.push_back(NoteBook());
    notes.back().updateNote(second,minutes,hour,month,day,year,content);
}

void update(std::vector<std::string>& cmds) {
    if(cmds.size() != 3){
        fprintf(stderr,"%s\n","update format: update#idx#content");
        return;
    }
    uint32_t idx = std::stoi(cmds[1]);
    const char* content = cmds[2].c_str();
    
    if(idx >= notes.size()) {
        errQuit("invalid idx!");
    }

    notes[idx].updateNote(content);
}

void show(std::vector<std::string>& cmds) {
    if(cmds.size() != 2){
        fprintf(stderr,"%s\n","show format: show#idx");
        return;
    }

    uint32_t idx = std::stoi(cmds[1]);
    
    if(idx >= notes.size()) {
        errQuit("invalid idx!");
    }

    const char* content = notes[idx].content_;

    printf("%d.%d.%d %d:%d:%d\n",notes[idx].year_,notes[idx].month_,notes[idx].day_,
                                notes[idx].hour_,notes[idx].minutes_,notes[idx].second_);
    write(1,content,strlen(content));
    puts("");
}

void delete_(std::vector<std::string>& cmds) {
    if(cmds.size() != 2){
        fprintf(stderr,"%s\n","delete format: delete#idx");
        return;
    }

    uint32_t idx = std::stoi(cmds[1]);
    
    if(idx >= notes.size()) {
        errQuit("invalid idx!");
    }

    uint64_t hash = (uint64_t)notes[idx].year_ << 40 |
                (uint64_t)notes[idx].month_ << 32 |
                notes[idx].day_ << 24 |
                notes[idx].hour_ << 16 |
                notes[idx].minutes_ << 8 |
                notes[idx].second_;

    if(enc_info.find(hash) != enc_info.end()) {
        enc_info.erase(hash);
    }

    if(note_hash.find(hash) != note_hash.end()) {
        note_hash.erase(hash);
    }

    notes.erase(notes.begin() + idx);
}

void encrypt(std::vector<std::string>& cmds) {
    if(cmds.size() != 4){
        fprintf(stderr,"%s\n","encrypt format: encrypt#idx#offset#length");
        return;
    }

    uint32_t idx = std::stoi(cmds[1]);
    uint32_t offset = std::stoi(cmds[2]);
    uint32_t length = std::stoi(cmds[3]);

    if(idx >= notes.size()) {
        errQuit("invalid idx!");
    }

    uint32_t content_length = strlen(notes[idx].content_);
    char* content = notes[idx].content_;

    if(offset >= content_length || length > content_length || (offset + length > content_length)) {
        errQuit("invalid offset or length!");
    }

    uint64_t hash = (uint64_t)notes[idx].year_ << 40 |
                    (uint64_t)notes[idx].month_ << 32 |
                    notes[idx].day_ << 24 |
                    notes[idx].hour_ << 16 |
                    notes[idx].minutes_ << 8 |
                    notes[idx].second_;

    enc* enc_ = (enc*)malloc(sizeof(enc));
    enc_->offset = offset;
    enc_->length = length;
    enc_->original = (char*)calloc(length+1,1);
    memcpy(enc_->original,content + offset,length);
    enc_info[hash].push_back(enc_);

    for(int i = 0;i < length;i++) {
        content[offset + i] ^= getRandom();
    }

    notes[idx].is_encrypt_ = true;

    printf("encrypt success\n");

}

void decrypt(std::vector<std::string>& cmds) {
    if(cmds.size() != 2){
        fprintf(stderr,"%s\n","decrypt format: decrypt#idx");
        return;
    }

    uint32_t idx = std::stoi(cmds[1]);
    
    if(idx >= notes.size()) {
        errQuit("invalid idx!");
    }

    if(notes[idx].is_encrypt_ == false){
        fprintf(stderr,"%s\n","this note is not encrypted");
        return;
    }

    uint64_t hash = (uint64_t)notes[idx].year_ << 40 |
                    (uint64_t)notes[idx].month_ << 32 |
                    notes[idx].day_ << 24 |
                    notes[idx].hour_ << 16 |
                    notes[idx].minutes_ << 8 |
                    notes[idx].second_;

    if(enc_info.find(hash) == enc_info.end()) {
        fprintf(stderr,"%s\n","hash error");
        return;
    }

    auto it = enc_info[hash].rbegin();
    for(;it != enc_info[hash].rend();it++) {
        memcpy(notes[idx].content_ + (*it)->offset,(*it)->original,(*it)->length);
    }

    enc_info.erase(hash);

    notes[idx].is_encrypt_ == false;

    printf("decrypt success\n");

}


int main(){
    init();

    char* buffer = new char[0x400];
    std::vector<std::string> cmds;

    while(1) {
        printf("input your test cmd:\n");
        memset(buffer,0,0x400);
        read_line(buffer,0x3ff);
        std::string line((const char*)buffer);
        split(line,cmds);
        
        if(cmds.size() < 1) {
            printf("no cmd!\n");
            continue;
        }

        if(m.find(cmds[0]) == m.end()) {
            printf("no operation named %s\n",cmds[0].c_str());
            continue;
        }

        switch(m[cmds[0]]) {
            case ADD:
                add(cmds);
                break;
            case UPDATE:
                update(cmds);
                break;
            case SHOW:
                show(cmds);
                break;
            case DELETE:
                delete_(cmds);
                break;
            case ENCRYPT:
                encrypt(cmds);
                break;
            case DECRYPT:
                decrypt(cmds);
                break;
            default:
                break;
        }

    }


    return 0;
}