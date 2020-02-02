#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <windows.h>
#include<tchar.h>

using namespace std;

typedef bool(*f_open_mp4)(unsigned int &,const char*, unsigned int,unsigned int,unsigned int);
typedef bool(*f_write_frame)(unsigned int,const char*,unsigned int);

int main()
{
    DWORD dwError = 0;
    HMODULE hmp4 = LoadLibrary(_T("D:\\workplace\\tz_mp4\\x64\\Debug\\tz_mp4.dll"));
    dwError = GetLastError();
    if (hmp4 != NULL) {
        f_open_mp4 open_mp4 = (f_open_mp4)GetProcAddress(hmp4, "open_mp4");
        f_write_frame write_frame = (f_write_frame)GetProcAddress(hmp4, "write_frame");

        unsigned int lPlayID = -1;
        bool ret = open_mp4(lPlayID, "D:\\workplace\\tz_mp4\\", 1920, 1080, 25);
        if (!ret) {
            return 0;
        }
        FILE *file = NULL;
        file = fopen("D:\\workplace\\tz_mp4\\linyuner.265", "rb");
        if (!file) {
            return 0;
        }
        char buff[2048];
        int count = 0;
        while (!feof(file)) {
            count = fread (buff, 1, 2048, file);
            write_frame(lPlayID, buff, count);
        }
        fclose(file);
        while(true) {
            Sleep(1000);
        };
    }

    printf("hello");
    system("pause");
    return 0;
}



