#include <iostream>
#include <fstream>
#include "Tongyi.h"
#include "Cidic.h"


using namespace std;

int main()
{
    Cidic dict;
    dict.loadDictFile("./dic/ch_en.txt");

    Tongyi tongyi;
    tongyi.loadDict(&dict);

    ifstream file;
    char bufferIn[1024];
    file.open("./dic/sample_en.txt");
    if (!file.is_open())
    {
        cout << "No input file detected. Exit." << endl;
        return 0;
    }
    file.get(bufferIn, 1023);
    file.close();

    char* bufferOut;
    bufferOut = tongyi.translate(bufferIn);
    cout << "Source text:" << endl << bufferIn << endl;
    cout << "Target text:" << endl << bufferOut << endl;
    delete bufferOut;
    return 0;
}
