#include <malloc.h>
#include <string>
#include <iostream>
#include "mpi.h"
#define DEBUG_OUTPUT(X) X

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
ostream& operator<<(std::ostream& stream, const LongInt& x);



class LongInt {
public:
    unsigned int* n;
    unsigned short size;

private:
    void Add(uint64_t x, int pos) {
        uint64_t int_max = (uint64_t)UINT32_MAX + (uint64_t)1;

        uint64_t y = this->n[pos];
        uint64_t xy = x + y;
        this->n[pos] = xy % int_max;
        if (xy / int_max != 0) this->Add(xy / int_max, pos + 1);
    }

public:
    LongInt() {
        size = 0;
        n = nullptr;
    }

    LongInt(unsigned short s) {
        n = (unsigned int*)calloc(s, 32);
        size = s;
    }

    LongInt(unsigned int* buff, int k) {
        do k--; while (buff[k] == 0);
        k++;
        n = (unsigned int*)calloc(k, sizeof(unsigned int));
        for (int i = 0; i < k; i++) {
            n[i] = buff[i];
        }
        size = k;
    }

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
    int N;
    int max_len = 0;
    int ProcRank, ProcNum;
    MPI_Status Status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);

    string* s = new string[0];
    LongInt* numbers = new LongInt[0];


    //Входные данные
    if (ProcRank == 0) {

        N = 9;
        string* s = new string[N]{
            "3000000000000000000000000000000000000000000000000000000" ,
            "4000000000000000000000000000000000000000000000000000000",
            "123","456","123456789","99999999999999",
            "7878878787878787","88888888888888888","77777777777777777777"
        };
        numbers = new LongInt[N];
        for (int i = 0; i < N; i++) {
            numbers[i] = LongInt(s[i]);

            DEBUG_OUTPUT
            (cout << "SIZE : " << numbers[i].size << endl;
            cout << "INT : " << numbers[i] << endl;
            cout << "DEC : ";
            print_longint(numbers[i]);
            cout << endl;)

                max_len += numbers[i].size;
        }
        delete[] s;
    }

    //Рассылка переменных
    MPI_Bcast(&N, 1, MPI_INT32_T, 0, MPI_COMM_WORLD);
    //cout << "I have N = " << N << endl;

    MPI_Bcast(&max_len, 1, MPI_INT32_T, 0, MPI_COMM_WORLD);
    //cout << "I have max_len = " << max_len << endl;

    int numbers_on_proc = N / ProcNum;
    if (N % ProcNum != 0) numbers_on_proc++;
    //cout << "I have n / proc = " << numbers_on_proc << endl;


    //Создаём тип данных MPI
    MPI_Datatype MPI_LONGINT_T;
    MPI_Type_contiguous(max_len, MPI_UINT32_T, &MPI_LONGINT_T);
    MPI_Type_commit(&MPI_LONGINT_T);

    int type_size;
    MPI_Pack_size(1, MPI_LONGINT_T, MPI_COMM_WORLD, &type_size);
    //cout << "SIZE = " << type_size << endl;

    int ints_in_type = type_size / sizeof(unsigned int);


    //Распределение чисел по процессам
    unsigned int* recv_nums = (unsigned int*)calloc(numbers_on_proc, type_size);
    unsigned int* send_nums = nullptr;
    if (ProcRank == 0) {
        for (int k = 0; k < ProcNum; k++) {
            send_nums = (unsigned int*)calloc(N, type_size);

            int pos = 0;
            for (int i = 0; i < N; i++) {
                DEBUG_OUTPUT(cout << " i = " << i << endl;)
                    pos = type_size * i;
                //cout << "pos = " << pos << endl;
                MPI_Pack(numbers[i].n, numbers[i].size, MPI_UINT32_T, send_nums, N * type_size, &pos, MPI_COMM_WORLD);
            }
        }
    }
    if (ProcRank == 0) {
        for (int i = 0; i < N; i++) {
            free(numbers[i].n);
        }
    }


    MPI_Scatter(send_nums, numbers_on_proc, MPI_LONGINT_T, recv_nums, numbers_on_proc, MPI_LONGINT_T, 0, MPI_COMM_WORLD);


    if (ProcRank == 0) {
        free(send_nums);
    }

    //Обработка полученных чисел
    DEBUG_OUTPUT
    (cout << "RECV NUMS FOR " << ProcRank << " PROCESS :" << endl;
    for (int i = 0; i < numbers_on_proc * type_size / sizeof(unsigned int); i++) {
        cout << recv_nums[i] << '\t';
        if ((i + 1) % (type_size / sizeof(unsigned int)) == 0) cout << endl;
    }cout << endl;)

    LongInt* numbers_to_multiply = new LongInt[numbers_on_proc];

    for (int i = 0; i < numbers_on_proc; i++) {
        numbers_to_multiply[i] = LongInt(&recv_nums[i * ints_in_type], max_len);
    }
    free(recv_nums);

    DEBUG_OUTPUT(
    cout << "LONGINTS FOR " << ProcRank << " PROCESS :" << endl;
    for (int i = 0; i < numbers_on_proc; i++)
        cout << numbers_to_multiply[i] << endl;
    )




    //Перемножение полученных чисел
    LongInt prod = LongInt("1");
    for (int i = 0; i < numbers_on_proc; i++) {
        prod = prod * numbers_to_multiply[i];
    }
    for (int i = 0; i < numbers_on_proc; i++) {
        free(numbers_to_multiply[i].n);
    }
    DEBUG_OUTPUT(cout << "PART PROD FOR " << ProcRank << " PROCESS :" << endl
        << "INT : " << prod << endl
        << "DEC : "; print_longint(prod);)


        //Сбор Частичных произведений
    int buff_size = 0;
    if (ProcRank == 0) buff_size = ints_in_type * ProcNum;
    unsigned int* prod_buff = (unsigned int*)calloc(buff_size, sizeof(unsigned int));

    MPI_Gather(prod.n, 1, MPI_LONGINT_T, prod_buff, 1, MPI_LONGINT_T, 0, MPI_COMM_WORLD);
    free(prod.n);

    DEBUG_OUTPUT(if (ProcRank == 0) {
        cout << "GATHER RESULT : " << endl;
        for (int i = 0; i < ints_in_type * ProcNum; i++) cout << prod_buff[i] << '\t'; cout << endl;
    })


    //Считаем общее произведение
    LongInt global_prod = LongInt("1");
    if (ProcRank == 0) {
        for (int i = 0; i < ProcNum; i++) {
            LongInt x = LongInt(&prod_buff[i * ints_in_type], max_len);
            global_prod = global_prod * x;
            free(x.n);
        }
    }

    free(prod_buff);

    if (ProcRank == 0) cout << "ANSWER =\t"; print_longint(global_prod);
    free(global_prod.n);
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

    delete[] b;
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

    free(r);
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
    return size;
}

void print_longint(LongInt x) {
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

    delete[] str_numbers;

    reverse(result.begin(), result.end());
    cout << result << endl;
}