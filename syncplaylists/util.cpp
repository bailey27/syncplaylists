/*
syncplaylists : Copies music files from specified iTunes playlists to specfied
                directory and writes .m3u playlist files.  Deletes all music
                and .m3u files that are not specified in the playlists.

Copyright (C) 2016-2020 Bailey Brown (github.com/bailey27/cppcryptfs)

cppcryptfs is based on the design of gocryptfs (github.com/rfjakob/gocryptfs)

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <string>
#include <vector>
#include <iostream>

#include <winver.h>

#pragma comment( lib, "version" )

#include "util.h"


namespace syncplaylists {

    namespace util {

        using namespace std;       

        const char* unicodeToUtf8(const wchar_t* unicode_str, string& storage)
        {

            storage.clear();

            auto len = ::WideCharToMultiByte(CP_UTF8, 0, unicode_str, -1, NULL, 0, NULL, NULL);

            if (len == 0)
                return NULL;

            // len includes space for null char
            vector<char> utf8(len);

            if (::WideCharToMultiByte(CP_UTF8, 0, unicode_str, -1, &utf8[0], len, NULL, NULL) != len) {
                return nullptr;
            }

            storage = &utf8[0];

            return &storage[0];
        }

        void printErr(const wstring& ws)
        {
            string s;

            if (unicodeToUtf8(ws.c_str(), s)) {
                cerr << s << endl;
            } else {
                cerr << "unable to convert string" << endl;
            }
        }

        void printOut(const wstring& ws)
        {
            string s;

            if (unicodeToUtf8(ws.c_str(), s)) {
                cout << s << endl;
            } else {
                cerr << "unable to convert string" << endl;
            }
        }

        void throwIfFalse(bool ok, const wstring& mes)
        {
            if (!ok) {
                string s;
                if (!unicodeToUtf8(mes.c_str(), s))
                    throw(std::exception("error occured, unable to convert message to utf8"));
                else
                    throw(std::exception(s.c_str()));
            }
        }

        wstring getFilename(const wstring& path)
        {
            if (path.length() < 1)
                return L"";

            auto p = ::wcsrchr(path.c_str(), L'\\');

            if (!p)
                return path;

            return p + 1;
        }

        wstring getExtension(const wstring& filename)
        {
            auto pdot = ::wcsrchr(filename.c_str(), L'.');

            if (!pdot)
                return L"";

            return pdot + 1;
        }

        bool GetProductVersionInfo(wstring& strProductName, wstring& strProductVersion,
                wstring& strLegalCopyright, HMODULE hMod)
        {

            TCHAR fullPath[MAX_PATH + 1];
            *fullPath = L'\0';
            if (!GetModuleFileName(hMod, fullPath, MAX_PATH)) {
                return false;
            }
            DWORD dummy = 0;
            DWORD vSize = GetFileVersionInfoSize(fullPath, &dummy);
            if (vSize < 1) {
                return false;
            }

            auto versionResourceStorage = vector<char>(vSize);

            void* pVersionResource = &versionResourceStorage[0];            

            if (!GetFileVersionInfo(fullPath, NULL, vSize, pVersionResource)) {
                return false;
            }

            // get the name and version strings
            LPVOID pvProductName = NULL;
            unsigned int iProductNameLen = 0;
            LPVOID pvProductVersion = NULL;
            unsigned int iProductVersionLen = 0;
            LPVOID pvLegalCopyright = NULL;
            unsigned int iLegalCopyrightLen = 0;

            struct LANGANDCODEPAGE {
                WORD wLanguage;
                WORD wCodePage;
            } *lpTranslate;

            // Read the list of languages and code pages.
            unsigned int cbTranslate;
            if (!VerQueryValue(pVersionResource,
                TEXT("\\VarFileInfo\\Translation"),
                (LPVOID*)&lpTranslate,
                &cbTranslate)) {

                return false;
            }

            if (cbTranslate / sizeof(struct LANGANDCODEPAGE) < 1) {
                return false;
            }

            wstring lang;

            WCHAR buf[16];

            // use the first language/codepage;

            wsprintf(buf, L"%04x%04x", lpTranslate->wLanguage, lpTranslate->wCodePage);

            lang = buf;

            // replace "040904e4" with the language ID of your resources
            if (!VerQueryValue(pVersionResource, (L"\\StringFileInfo\\" + lang + L"\\ProductName").c_str(), &pvProductName, &iProductNameLen) ||
                !VerQueryValue(pVersionResource, (L"\\StringFileInfo\\" + lang + L"\\ProductVersion").c_str(), &pvProductVersion, &iProductVersionLen) ||
                !VerQueryValue(pVersionResource, (L"\\StringFileInfo\\" + lang + L"\\LegalCopyright").c_str(), &pvLegalCopyright, &iLegalCopyrightLen))
            {
                return false;
            }

            if (iProductNameLen < 1 || iProductVersionLen < 1 || iLegalCopyrightLen < 1) {
                return false;
            }

            strProductName = (LPCTSTR)pvProductName;
            strProductVersion = (LPCTSTR)pvProductVersion;
            strLegalCopyright = (LPCTSTR)pvLegalCopyright;

            return true;
        }

    } // namespace util
} // namespace syncplaylists
