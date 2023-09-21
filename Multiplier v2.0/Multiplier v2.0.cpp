#include <iostream>
#include <bitset>
#include <climits>
#include <iomanip>
#include <intrin.h>
#include <string>

using namespace std;

union itof
{
    unsigned int input;
    float output;
};

union ftoi
{
    float input;
    unsigned int output;
};

int manLength = 23;
int expLength = 8;

bool info = 1;

struct float_num
{
    float_num(unsigned int numVal)
    {        
        num = 0;
        numBin = numVal;        
        numSign = numVal >> (expLength + manLength);
        numExp = ((numVal >> manLength) & (0xffff >> (16 - expLength)));
        numMan = numVal & (0x7FFFFFF >> (27 - manLength));
        if ((numExp == 0) && (numMan == 0)) DINN[3] = 1;
        else if ((numExp == (pow(2, expLength) - 1)) && (numMan == 0)) DINN[1] = 1;
        else if ((numExp == (pow(2, expLength) - 1)) && (numMan > 0)) DINN[2] = 1;
        else if ((numExp == 0) && (numMan > 0)) DINN[0] = 1;

        unsigned int expBuf = numExp - (pow(2, (expLength - 1)) - 1);
        itof a;
        expBuf += 127;
        unsigned int numBuf = (expBuf << 23);
        if (manLength <= 23) numBuf += (numMan << (23 - manLength));
        else numBuf += (numMan >> (manLength - 23));
        numBuf += (numSign << 31);
        a.input = numBuf;
        num = a.output;
    }
    float_num(float numVal)
    {
        num = numVal;
        numBin = *reinterpret_cast<unsigned int*>(&numVal);
        numSign = numBin >> (expLength + manLength);
        numExp = ((numBin >> manLength) & (0xffff >> (16 - expLength)));
        numMan = numBin & (0x7FFFFFF >> (27 - manLength));
        if ((numExp == 0) && (numMan == 0)) DINN[3] = 1;
        else if ((numExp - (pow(2, (expLength - 1))) > (pow(2, (expLength - 1)))) && (numMan == 0)) DINN[1] = 1;
        else if ((numExp - (pow(2, (expLength - 1))) > (pow(2, (expLength - 1)))) && (numMan > 0)) DINN[2] = 1;
        else if ((numExp == 0) && (numMan > 0)) DINN[0] = 1;
    }

    float num;
    unsigned int numBin;
    unsigned int numSign;
    int numExp;
    unsigned int numMan;
    bool DINN[4] = { 0,0,0,0 };   // DINN (D - Denormalized, I - inf, N - NaN, N - NUL)

    void print(char mode)
    {
        bitset<32> numBinBit(numBin);
        if (mode == 'r') cout << "Res: ";
        else if (mode == 'a') cout << "A:   ";
        else if (mode == 'b') cout << "B:   ";
        cout << hex << "HEX: " << numBin << dec << "\t";
        cout << "BIN: " << numBinBit[manLength + expLength] << " ";
        for (int i = (manLength + expLength - 1); i > (manLength - 1); i--)
        {
            cout << numBinBit[i];
        }
        cout << " ";
        for (int i = (manLength - 1); i >= 0; i--)
        {
            cout << numBinBit[i];
        }
        if (DINN[2]) cout << " (NaN)" << "\t";
        else if (DINN[1]) cout << " (inf)" << "\t";
        else if (DINN[3]) cout << " (NUL)" << "\t";
        else if (DINN[0]) cout << " (Denormalized)" << "\t";
        else cout << "\t";

        if (DINN[1]) num = INFINITY;
        else if ((numExp - ((pow(2, (expLength - 1))) - 1)) > 127) num = INFINITY;
        cout << setprecision(15) << "DEC(fp32): " << num << endl << endl;
    }
    
};

void helpManPrint(unsigned int& man)
{
    bitset<32> manbit(man);
    cout << '1' << '.';
    for (int i = manLength - 1; i >= 0; i--)
    {
        cout << manbit[i];
    }
}
void manMultPrint(unsigned int manA, unsigned int manB, unsigned long long manRes, bool& addExp, bool& rounding)
{
    bitset<60> manRbit(manRes);

    cout << "Man.A x Man.B: ";
    helpManPrint(manA);
    cout << " x ";
    helpManPrint(manB);
    cout << " = ";
    if (addExp) cout << "10" << '.';
    else cout << '1' << '.';
    for (int i = ((manLength * 2) - 1); i >= 0; i--)
    {
        cout << manRbit[i];
    }
    cout << endl;
}

unsigned int manMult(unsigned int manA, unsigned int manB, bool &addExp, bool& rounding)
{
    
    unsigned long long buf = 0;
    unsigned int result = 0;
    manA = manA | (0x8000000 >> (27 - manLength)); manB = manB | (0x8000000 >> (27 - manLength));
    unsigned long long bufB = manB;
    unsigned long long bufA = manA;
    buf = bufA * bufB;
    /*while (manB != 0)
    {
        if (manB & 1)
            buf += bufA;

        manB >>= 1;
        bufA <<= 1;
    }*/

    if (buf & (0x80000000000000ul >> (56 - (manLength + 1) * 2)))
    {
        buf >>= 1;
        addExp = 1;
    }
    buf &= (0b00111111111111111111111111111111111111111111111111111111ul >> (56 - (manLength + 1) * 2));

    if (info) manMultPrint(manA, bufB, buf, addExp, rounding);
    if (((buf & (0b1000000000000000000000000000 >> (28 - (manLength + 1)))) && (buf & (0b0100000000000000000000000000 >> (28 - (manLength + 1))))) || ((buf & 0b0100000000000000000000000000 >> (28 - (manLength + 1)))) && (buf & (0b0011111111111111111111111111 >> (28 - (manLength + 1)))))
    {
        buf += (0b1000000000000000000000000000 >> (28 - (manLength + 1)));
        rounding = 1;
    }

    result = buf >> manLength;
    if (info) 
    {
        cout << "Normalised: ";
        bitset<60> manbitCut(buf);
        cout << '1' << '.';
        for (int i = ((manLength * 2) - 1); i > manLength - 1; i--)
        {
            cout << manbitCut[i];
        }
        cout << " | ";
        for (int i = manLength - 1; i >= 0; i--)
        {
            cout << manbitCut[i];
        }
        cout << endl;
        cout << "Rounded: ";
        helpManPrint(result);
        cout << "\tOne bit added: " << boolalpha << rounding << noboolalpha << endl;
    }
    return result;
}

void expAddPrint(int& expA, int& expB, int& expRes, bool& addExp)
{
    if (addExp) cout << "Exp.A + Exp.B + overflow: " << expA << " + " << expB << " = " << expRes << " + 1 = " << expRes + 1;
    else cout << "Exp.A + Exp.B: " << expA << " + " << expB << " = " << expRes;
    cout << endl << endl;
}
int expAdd(int &expA, int &expB, bool &addExp)
{
    expA -= (pow(2,(expLength - 1)) - 1); expB -= (pow(2, (expLength - 1)) - 1);
    int expRes = (expA + expB);
    if (info) expAddPrint(expA, expB, expRes, addExp);
    if (addExp) expRes++;
    return expRes + (pow(2, (expLength - 1)) - 1);
}

unsigned int addRes(unsigned int mantissa, int exponent, unsigned int sign)
{
    unsigned int result = 0;
    result += mantissa;
    result += exponent << manLength;
    result += sign << (manLength + expLength);
    return result;
}

unsigned int mul(unsigned int num1, unsigned int num2)
{

    unsigned int numSign1 = num1 >> (expLength + manLength);
    int numExp1 = ((num1 >> manLength) & (0xffff >> (16 - expLength)));
    unsigned int numMan1 = num1 & (0x7FFFFFF >> (27 - manLength));

    bool DINN1[4] = { 0,0,0,0 };
    if ((numExp1 == 0) && (numMan1 == 0)) DINN1[3] = 1;
    else if ((numExp1 == (pow(2, expLength) - 1)) && (numMan1 == 0)) DINN1[1] = 1;
    else if ((numExp1 == (pow(2, expLength) - 1)) && (numMan1 > 0)) DINN1[2] = 1;
    else if ((numExp1 == 0) && (numMan1 > 0)) DINN1[0] = 1;


    unsigned int numSign2 = num2 >> (expLength + manLength);
    int numExp2 = ((num2 >> manLength) & (0xffff >> (16 - expLength)));
    unsigned int numMan2 = num2 & (0x7FFFFFF >> (27 - manLength));

    bool DINN2[4] = { 0,0,0,0 };
    if ((numExp2 == 0) && (numMan2 == 0)) DINN2[3] = 1;
    else if ((numExp2 == (pow(2, expLength) - 1)) && (numMan2 == 0)) DINN2[1] = 1;
    else if ((numExp2 == (pow(2, expLength) - 1)) && (numMan2 > 0)) DINN2[2] = 1;
    else if ((numExp2 == 0) && (numMan2 > 0)) DINN2[0] = 1;


    bool addExp = 0;
    bool rounding = 0;
    if (DINN1[0] or DINN2[0])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        int expRes = 0;
        unsigned int manRes = manMult(numMan1, numMan2, addExp, rounding);
        
        return addRes(manRes, expRes, signRes);
    }
    else if (DINN1[1] or DINN2[1])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        int expRes = (pow(2, (expLength)) - 1);
        unsigned int manRes = 0;

        return addRes(manRes, expRes, signRes);
    }
    else if (DINN1[2] or DINN2[2])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        int expRes = (pow(2, (expLength)) - 1);
        unsigned int manRes = manMult(numMan1, numMan2, addExp, rounding);

        return addRes(manRes, expRes, signRes);
    }
    else if (DINN1[3] or DINN2[3])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        int expRes = 0;
        unsigned int manRes = 0;

        return addRes(manRes, expRes, signRes);
    }
    else
    {
        unsigned int signRes = numSign1 ^ numSign2;
        if (info) cout << "sign.A xor sign.B: " << numSign1 << " xor " << numSign2 << " = " << signRes << endl;
        unsigned int manRes = manMult(numMan1, numMan2, addExp, rounding);
        //manMultPrint(numMan1, numMan2, manRes, addExp, rounding);
        int expRes = expAdd(numExp1, numExp2, addExp);
        if (expRes > (pow(2, expLength) - 1))
        {
            expRes = (pow(2, (expLength)) - 1);
            manRes = 0;
        }

        return addRes(manRes, expRes, signRes);
    }
    
}

void enterNumbers(unsigned int& number1, unsigned int& number2)
{
    if ((manLength == 23) && (expLength == 8))
    {
        char type1;
        char type2;
        cout << "Ввод операндов A и B в формате: тип(h/b/d) число тип(h/b/d) число ( h - HEX, b - BIN, d - DEC (float) )" << endl;

        cin >> type1;
        switch (type1)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number1 = stoul(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number1 = stoul(buf, 0, 2);
            break;
        }
        case 'd':
        {
            float buf;
            ftoi a;
            cin >> buf;
            a.input = buf;
            number1 = a.output;
            break;
        }
        }

        cin >> type2;
        switch (type2)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number2 = stoul(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number2 = stoul(buf, 0, 2);
            break;
        }
        case 'd':
        {
            float buf;
            ftoi a;
            cin >> buf;
            a.input = buf;
            number2 = a.output;
            break;
        }
        }

        cout << endl;
    }
    else
    {
        char type1;
        char type2;
        cout << "Ввод операндов A и B в формате: тип(h/b) число тип(h/b) число ( h - HEX, b - BIN )" << endl;

        cin >> type1;
        switch (type1)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number1 = stoul(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number1 = stoul(buf, 0, 2);
            break;
        }
        }

        cin >> type2;
        switch (type2)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number2 = stoul(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number2 = stoul(buf, 0, 2);
            break;
        }
        }


        cout << endl;
    }
}
void enterSettings(int& manLength, int& expLength)
{
    cout << "*Округление выполняется к ближайшему большому целому числу\nмаксимальная и минимальная экспонента = 16 и 2, максимальная и минимальная мантисса = 28 и 3\nчисло не должно быть больше 32 бит*" << endl << endl;
    cout << "Введите длинну экспоненты и мантиссы (с мнимой единицей) через пробел" << endl;
    cin >> expLength;
    cin >> manLength;
    manLength--;
}

void checker(float num1, float num2, float numRes)
{
    float numResC = num1 * num2;
    if (numResC == numRes) cout << "CHECKER: CORRECT" << endl << endl;
    else
    {
        cout << "CHECKER: ERROR" << endl;
        cout << "RIGHT IS: " << numRes << endl << endl;
    }
}

int main()
{
    setlocale(LC_ALL, "rus");

    enterSettings(manLength, expLength);
    unsigned int number1 = 0;
    unsigned int number2 = 0;
    enterNumbers(number1, number2);
                   //se------em---------------------m
    float_num num1(number1);
    if (info) num1.print('a');
    float_num num2(number2);
    if (info) num2.print('b');
    float_num num3(mul(num1.numBin, num2.numBin));
    if (info) num3.print('r');

    if (info and manLength == 23 and expLength == 8) checker(num1.num, num2.num, num3.num);
    system("pause");
}

