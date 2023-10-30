#include <malloc.h>
#include <string>
#include <iostream>
#include "mpi.h"

using namespace std;

class LongInt;
string multiply(string a, string b);
string sum(string a, string s, int pos);
string add(string s, int pos);
void print_longint(LongInt x);
string substract(string a, string b, bool& res);

bool* ToBin(string s, unsigned short size);
unsigned int* ToInt(bool* bin, unsigned short size);

unsigned int* StringToLongint(string s, unsigned short& size);
unsigned short GetLongintSize(string s);
string trim_zeros(string s);



class LongInt {
public:
    unsigned int* n;
    unsigned short size;

private:
    LongInt(unsigned short s) {
        n = (unsigned int*)calloc(s, 32);
        size = s;
    }

    void Add(uint64_t x, int pos) {
        uint64_t int_max = (uint64_t)UINT32_MAX + (uint64_t)1;

        uint64_t y = this->n[pos];
        uint64_t xy = x + y;
        this->n[pos] = xy % int_max;
        if (xy / int_max != 0) this->Add(xy / int_max, pos + 1);
    }

public:

    LongInt(string s) {
        unsigned short& r = this->size;
        n = StringToLongint(s, size);
    }

    LongInt operator * (const LongInt& a) {
        LongInt s = LongInt(a.size + this->size);
        uint64_t int_max = (uint64_t)UINT32_MAX + (uint64_t)1;
        for (size_t i = 0; i < a.size; i++) {
            for (size_t j = 0; j < this->size; j++) {
                uint64_t x = a.n[i];
                uint64_t y = this->n[j];
                uint64_t xy = x * y;


                int pos = i + j;
                s.Add(xy % int_max, pos);
                if (xy / int_max != 0) s.Add(xy / int_max, pos + 1);
            }
        }
        return s;
    }



};
ostream& operator<<(std::ostream& stream, const LongInt& x)
{
    for (int i = 0; i < x.size; i++)
        stream << x.n[i] << '\t';
    return stream;
}


int main(int argc, char* argv[])
{
    int ProcRank, ProcNum;
    MPI_Status Status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);

    if (ProcRank == 0) {
        string s1 = "3000000000000000000000000000000000000000000000000000000";
        string s2 = "4000000000000000000000000000000000000000000000000000000";

        LongInt x = LongInt(s1);
        LongInt y = LongInt(s2);

        LongInt xy = x * y;

        print_longint(x);
        print_longint(y);
        cout << "RESULT :   ";
        print_longint(xy);
    }
    MPI_Finalize();
}

string multiply(string a, string b) {
    string s = "0";
    for (size_t i = 0; i < a.length(); i++) {
        for (size_t j = 0; j < b.length(); j++) {
            int x = a[i] - '0';
            int y = b[j] - '0';

            int r = x * y;
            string xy = to_string(r);
            reverse(xy.begin(), xy.end());
            s = sum(xy, s, i + j);
        }
    }
    return s;
}

string sum(string a, string s, int pos) {
    if (a == "0") return s;
    for (size_t i = 0; i < a.length(); i++) {
        int x = a[i] - '0';
        if (pos + i >= s.length()) {
            int k_zeros = pos + i - s.length();
            for (int k = 0; k <= k_zeros; k++) {
                s += '0';
            }
        }
        int y = s[pos + i] - '0';

        int r = x + y;
        string xy = to_string(r);
        if (xy.length() == 1)
            s[pos + i] = xy[0];
        else {
            s[pos + i] = xy[1];
            s = add(s, pos + i + 1);
        }
    }
    return s;
}

string add(string s, int pos) {
    if (pos >= s.length()) {
        s = s + '0';
    }
    int x = s[pos] - '0';
    if (s[pos] == '9') {
        s[pos] = '0';
        s = add(s, pos + 1);
    }
    else {
        s[pos] ++;
    }
    return s;
}

string substract(string a, string b, bool& res) {

    bool borrow = false;
    string rasnost = "";
    a = trim_zeros(a);
    b = trim_zeros(b);
    if (a.length() < b.length()) {
        res = false;
        return "";
    }
    size_t i = 0;
    while ((i < b.length()) || borrow) {
        if (i >= a.length()) {
            res = false;
            return "";
        }
        int x = a[i] - '0';
        int y;
        if (i < b.length()) {
            y = b[i] - '0';
        }
        else y = 0;


        if (borrow) x -= 1;
        if (x < y) {
            x += 10;
            borrow = true;
        }
        else borrow = false;

        int r = x - y;
        rasnost += to_string(r);
        i++;
    }
    res = true;
    return rasnost;
}

string trim_zeros(string s) {
    string new_s;
    //if ((s == "0") || (s == "") || (s == "00") || (s == "000")|| (s == "0000")||(s == "00000")) return "0";

    int i = s.length();
    do {
        if ((i == 0) && (s[i] == '0')) return "0";
        i--;

    } while (s[i] == '0');

    new_s = s.substr(0, i + 1);
    return new_s;
}



bool* ToBin(string s, unsigned short size) {
    bool* res = (bool*)malloc(32 * size);
    string* b = new string[32 * size];
    b[32 * size - 1] = "1";
    for (int i = 32 * size - 2; i >= 0; i--) {
        b[i] = multiply(b[i + 1], "2");
    }

    for (int i = 0; i < 32 * size; i++) {
        bool can_substract;
        bool& p = can_substract;
        string temp = substract(s, b[i], p);
        if (can_substract) {
            res[i] = true;
            s = temp;
        }
        else {
            res[i] = false;
        }
    }
    return res;
}

unsigned int* ToInt(bool* bin, unsigned short size) {
    unsigned int* result = (unsigned int*)malloc(4 * size);
    for (int i = 0; i < size; i++) {
        result[i] = 0;
    }
    reverse(bin, bin + 32 * size);
    for (int k = 0; k < size; k++) {
        for (int i = k * 32; i < (k + 1) * 32; i++) {
            result[k] += bin[i] * powl(2, i % 32);
        }
    }
    return result;
}

unsigned int* StringToLongint(string s, unsigned short& size) {
    reverse(s.begin(), s.end());
    size = GetLongintSize(s);
    bool* r = ToBin(s, size);
    unsigned int* result = ToInt(r, size);
    return result;
}

unsigned short GetLongintSize(string s) {
    string int_max = "4294967296";   reverse(int_max.begin(), int_max.end());
    string current_max = "4294967296";   reverse(current_max.begin(), current_max.end());
    unsigned short size = 0;
    bool can_substract = true;
    bool& p = can_substract;
    while (can_substract) {
        substract(s, current_max, p);
        current_max = multiply(current_max, int_max);

        size++;
    }
    cout << "size = " << size << endl;

    return size;

}

void print_longint(LongInt x) {
    cout << x << endl;
    string int_max = "4294967296";       reverse(int_max.begin(), int_max.end());
    string current_max = "1";

    string* str_numbers = new string[x.size];
    string result = "0";

    for (int i = 0; i < x.size; i++) {
        str_numbers[i] = to_string(x.n[i]);
        reverse(str_numbers[i].begin(), str_numbers[i].end());

        str_numbers[i] = multiply(str_numbers[i], current_max);
        result = sum(str_numbers[i], result, 0);

        current_max = multiply(current_max, int_max);
    }

    reverse(result.begin(), result.end());
    cout << result << endl;
}