#include <cstdint>
#include <iostream>
#include <fstream>
#include <bitset>


#ifndef LENGTH
#define LENGTH 1
#endif
#define MAXLEN 100
#define STUDENT_ID_LAST_DIGIT 7



int16_t lab1(int16_t n) {
    // initialize
    if ((n & 1) == 0) n = -n;
    int16_t m = 0;
    n = ~n;
    // calculation
    do {
        ++m;
        n = n & (n - 1);
    } while (n);
    // return value
    return m + STUDENT_ID_LAST_DIGIT;
}

int16_t lab2(int16_t n) {
    // initialize
    int16_t r0 = 3, r1 = 2, r2 = n - 1, r6 = 0xfff;
    // calculation
    while (r2) {
        r0 += r0 + r1; r0 &= r6;
        if (r0 & 7) {
            int16_t r5 = r0, r4 = 0;
            while ((r5 -= 1000) >= 0) ;
            r5 += 1000;
            do { ++r4; r5 -= 100; } while (r5 >= 0);
            r5 += 100; r4 -= 9;
            if (r4) {
                r4 = 0;
                do { ++r4; r5 -= 10; } while (r5 >= 0);
                r5 += 2; r4 -= 9;
                if ((r5 != 0) & (r4 != 0)) {
                    --r2;
                    continue;
                }
            }
        }
        r1 = -r1;
        --r2;
    }
    // return value
    return r0;
}

int16_t my_strcmp(char a[], char b[]) {
    for (int i = 0; (a[i] != 0) | (b[i] != 0); ++i) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

void lab3(char s1[], char s2[], int input_cnt, char my_input[10][MAXLEN]) {
    int i = 0;
    if (my_strcmp(my_input[i++], s1)) {
        puts("righ");
        return;
    }
    puts("wron");
    puts(s2);
    if (my_strcmp(my_input[i++], s2)) {
        puts("righ");
        if (my_strcmp(my_input[i++], s1)) {
            puts("righ");
            return;
        }
        puts("wron");
        return;
    }
    puts("wron");
}

int16_t getshl(int n) {
    int16_t res = 1;
    for (int i = 0; i < n - 1; ++i) res += res;
    return res;
}

void PUT(int, int16_t*, int16_t&, int16_t&);

void REMOVE(int n, int16_t *memory, int16_t &count, int16_t &state) {
    if (n == 0) return;
    if (n == 1) {
        ++state;
        memory[count++] = state;
        return;
    }
    REMOVE(n - 2, memory, count, state);
    state |= getshl(n);
    memory[count++] = state;
    PUT(n - 2, memory, count, state);
    REMOVE(n - 1, memory, count, state);
}

void PUT(int n, int16_t *memory, int16_t &count, int16_t &state) {
    if (n == 0) return;
    if (n == 1) {
        --state;
        memory[count++] = state;
        return;
    }
    PUT(n - 1, memory, count, state);
    REMOVE(n - 2, memory, count, state);
    state &= ~getshl(n);
    memory[count++] = state;
    PUT(n - 2, memory, count, state);
}

int16_t lab4(int16_t *memory, int16_t n) {
    // initialize
    int16_t count = 0, state = 0;
    // calculation
    REMOVE(n, memory, count, state);
    // return value
    return count;
}


int main()
{
    std::fstream file;
    file.open("test.txt", std::ios::in);



    // lab1
    int16_t n = 0;
    std::cout << "===== lab1 =====" << std::endl;
    for (int i = 0; i < LENGTH; ++i) {
        file >> n;
        std::cout << lab1(n) << std::endl;
    }

    // lab2
    std::cout << "===== lab2 =====" << std::endl;
    for (int i = 0; i < LENGTH; ++i) {
        file >> n;
        std::cout << lab2(n) << std::endl;
    }

    // lab3
    std::cout << "===== lab3 =====" << std::endl;
    char passwd[MAXLEN]; char verify[MAXLEN];
    int input_cnt=-1;
    char my_input[10][MAXLEN];
    for (int i = 0; i < LENGTH; ++i) {
        file >> passwd >> verify;
        file >> input_cnt;
        for (int j=0; j< input_cnt; j++)
        {
            file >> my_input[j];
        }
        lab3(passwd, verify , input_cnt, my_input);
    }
    
    // lab4
    std::cout << "===== lab4 =====" << std::endl;
    int16_t memory[MAXLEN], move;
    for (int i = 0; i < LENGTH; ++i) {
        file >> n;
        int16_t state = 0;
        move = lab4(memory, n);
        for(int j = 0; j < move; ++j){
            std::cout << std::bitset<16>(memory[j]) << std::endl;
        }
    }



    return 0;
}