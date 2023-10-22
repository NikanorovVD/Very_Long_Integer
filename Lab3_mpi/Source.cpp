#include <malloc.h>
#include <string>
#include <iostream>
#include "mpi.h"

using namespace std;

string multiply(string a, string b);
string sum(string a, string s, int pos);
string add(string s, int pos);
void print_longint(unsigned int* buff);
string substract(string a, string b, bool& res);
void ToBin(string s, bool* res);
void ToInt(bool* res, unsigned int* result);

int main(int argc, char* argv[])
{
    string s = "12345678999827341230"; reverse(s.begin(), s.end());
    bool r[128];

    ToBin(s, r); 
    for (int i = 0; i < 128; i++)
        cout << r[i];
    cout << endl;

    unsigned int* result = (unsigned int*)malloc(16);
    ToInt(r, result);


    for (int i = 0; i < 4; i++)
        cout << result[i]<<endl;
    print_longint(result);

  /* string a = "123"; reverse(a.begin(), a.end());
    string b = "122";  reverse(b.begin(), b.end());
    bool r = true;
    bool& p = r;
    string s = substract(a, b, p); reverse(s.begin(), s.end());
    cout << r << endl;
    cout << s << endl;*/
    //unsigned int* buff1 = divide(a);
    //reverse(buff1, buff1 + 4);
    //print_longint(buff1);

    int ProcRank, ProcNum;
    MPI_Status Status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);

    MPI_Datatype longint;
    MPI_Type_contiguous(4, MPI_UNSIGNED, &longint);
    MPI_Type_commit(&longint);

    int size;
    MPI_Pack_size(4, MPI_UNSIGNED, MPI_COMM_WORLD, &size);


    unsigned int* buff = (unsigned int*)malloc(size);
    unsigned int* buff1 = (unsigned int*)malloc(size);

    unsigned int a = 0;
    unsigned int b = 0;
    unsigned int c = 0;
    unsigned int d = 0;


    int a_pos = 0;
    int b_pos = 4;
    int c_pos = 8;
    int d_pos = 12;


    if (ProcRank == 0) {
        a = 12354;
        b = 6848;
        c = 3000;
        d = 2500;

        MPI_Pack(&a, 1, MPI_UNSIGNED, buff, size, &a_pos, MPI_COMM_WORLD);
        MPI_Pack(&b, 1, MPI_UNSIGNED, buff, size, &b_pos, MPI_COMM_WORLD);
        MPI_Pack(&c, 1, MPI_UNSIGNED, buff, size, &c_pos, MPI_COMM_WORLD);
        MPI_Pack(&d, 1, MPI_UNSIGNED, buff, size, &d_pos, MPI_COMM_WORLD);

        for (int i = 1; i < ProcNum; i++)
            MPI_Send(buff, 1, longint, i, 0, MPI_COMM_WORLD);
    }

    if (ProcRank != 0) {
        MPI_Recv(buff1, 1, longint, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
        print_longint(buff1);
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
            s = s + "000";
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
        add(s, pos + 1);
    }
    else {
        s[pos] ++;
    }
    return s;
}

void print_longint(unsigned int* buff) {
    string S_0 = "1";
    string S_1 = "4294967296";                    reverse(S_1.begin(), S_1.end());
    string S_2 = "18446744073709551616";          reverse(S_2.begin(), S_2.end());
    string S_3 = "79228162514264337593543950336"; reverse(S_3.begin(), S_3.end());


    //a = 12354;
    //b = 6848;
    //c = 3000;
    //d = 2500;

    unsigned int a = buff[0];
    unsigned int b = buff[1];
    unsigned int c = buff[2];
    unsigned int d = buff[3];

    string as = to_string(a); reverse(as.begin(), as.end());
    string bs = to_string(b); reverse(bs.begin(), bs.end());
    string cs = to_string(c); reverse(cs.begin(), cs.end());
    string ds = to_string(d); reverse(ds.begin(), ds.end());


    string result = "0";
    S_0 = multiply(S_0, as);
    S_1 = multiply(S_1, bs);
    S_2 = multiply(S_2, cs);
    S_3 = multiply(S_3, ds);

    result = sum(S_0, result, 0);
    result = sum(S_1, result, 0);
    result = sum(S_2, result, 0);
    result = sum(S_3, result, 0);

    reverse(result.begin(), result.end());
    cout << result << endl;
}

string substract(string a, string b, bool& res) { 
//a - �����������
//b - �����������
//���������� �������� �� �������
bool borrow = false; //�������� �� 10 �� ����. �������
string rasnost = "";
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

void ToBin(string s, bool* res) {
    string b[128];
    b[127] = "1";
    for (int i = 126; i >=0; i--) {
        b[i] = multiply(b[i + 1], "2");
    }

    //for (int i = 0; i < 128; i++) {
        //reverse(b[i].begin(), b[i].end());
    //}

    for (int i = 0; i < 128; i++) {
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

}

unsigned int* divide(string delimoe) {
    unsigned long delitel = UINT32_MAX;
    unsigned int* result = (unsigned int*)malloc(16);
    for (int i = 0; i < 16; i++) {
        result[i] = 0;
    }
    int k = 0;
    string temp;
    while (true) {
        if (delimoe.length() < 10) { result[k] = stoull(delimoe); return result; }
        temp = delimoe.substr(0,10);
        delimoe = delimoe.substr(10, delimoe.length() - 10);
        unsigned long temp_int = stoull(temp);
        if (temp_int < delitel) {
            temp += delimoe[0];
            temp_int = stoull(temp);
            delimoe = delimoe.substr(1, delimoe.length() - 1);
        }
        unsigned int ostatok = temp_int % delitel;
        result[k] = ostatok;
        k++;
    }
}

void ToInt(bool* bin, unsigned int* result) {
    
    for (int i = 0; i < 4; i++) {
        result[i] = 0;
    }

    reverse(bin, bin + 128);
    
    for (int k = 0; k < 4; k++) {
        for (int i = k*32; i < (k+1)*32; i++) {
            result[k] += bin[i] * powl(2, i%32);
        }
    }
}

