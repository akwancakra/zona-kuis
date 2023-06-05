#include <iostream>
#include <string>
#include <sstream>

using namespace std;

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

int main()
{
    string input = "probstat-terbaru-bosque";
    string output = capitalizeString(input);

    cout << "Input: " << input << endl;
    cout << "Output: " << output << endl;

    return 0;
}
