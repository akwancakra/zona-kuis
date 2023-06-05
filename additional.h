#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <windows.h>

using namespace std;

bool isInteger(const string &input)
{
    for (char const &c : input)
    {
        if (isdigit(c) == 0)
            return false;
    }
    return true;
}

string capitalizeString(const string &input)
{
    string result;
    stringstream ss(input);

    string word;
    while (getline(ss, word, '-'))
    {
        if (!result.empty())
        {
            result += " ";
        }
        // Mengubah huruf pertama menjadi kapital
        word[0] = toupper(word[0]);
        result += word;
    }

    return result;
}

string removeQuotesAndFileExtension(const string filename)
{
    // Cari posisi tanda petik pertama dan terakhir dalam nama file
    size_t firstQuotePos = filename.find_first_of('"');
    size_t lastQuotePos = filename.find_last_of('"');

    if (firstQuotePos != string::npos && lastQuotePos != string::npos && firstQuotePos < lastQuotePos)
    {
        // Ambil substring di antara tanda petik pertama dan terakhir
        string nameWithoutQuotes = filename.substr(firstQuotePos + 1, lastQuotePos - firstQuotePos - 1);

        // Cari posisi titik terakhir dalam nama file
        size_t lastDotPos = nameWithoutQuotes.find_last_of('.');
        if (lastDotPos != string::npos)
        {
            // Ambil substring sebelum posisi titik terakhir
            string nameWithoutExtension = nameWithoutQuotes.substr(0, lastDotPos);
            return nameWithoutExtension;
        }

        // Jika tidak ada ekstensi, kembalikan string asli tanpa tanda petik
        return nameWithoutQuotes;
    }

    // Jika tidak ada tanda petik atau tanda petik tidak berpasangan, kembalikan string asli
    return filename;
}

void changeColor(int desiredColor)
{
    // 14 OREN
    // 13 UNGU
    // 12 MERAH
    // 11 CYAN
    // 10 HIJAU MUDA
    // 9 BIRU MUDA
    // 8 ABU
    // 7 PUTIH
    // 6 KUNING

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), desiredColor);
}