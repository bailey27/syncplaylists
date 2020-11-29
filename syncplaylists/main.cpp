/*
syncplaylists : Copies music files from specified iTunes playlists to specfied 
                directory and writes .m3u playlist files.  Deletes all music
                and .m3u files that are not specified in the playlists.

Copyright (C) 2020 Bailey Brown (github.com/bailey27/syncplaylists)

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

#include <iostream>
#include <string>
#include <clocale>
#include <unordered_set>
#include <unordered_map>
#include <unordered_set>
#include <shlwapi.h>

#pragma comment( lib, "shlwapi" )

#include "util.h"
#include "itunes.h"
#include "disk.h"

using namespace std;

using namespace syncplaylists::common;
using namespace syncplaylists::util;
using namespace syncplaylists::disk;
using namespace syncplaylists::itunes;


int wmain(int argc, const wchar_t *argv[])
{

    int rval = 0; 
    
    try {

        throwIfFalse(std::setlocale(LC_ALL, "en_US.UTF-8") != nullptr, L"unable to set locale");

        unordered_set<wstring> sync_playlists;
        wstring usbroot;
      
        if (argc < 3 || ::wcslen(argv[1]) < 3) {        
            wstring prodName, prodVer, prodCopyright;
            if (GetProductVersionInfo(prodName, prodVer, prodCopyright)) {
                printErr(prodName + L" version " + prodVer + L" " + prodCopyright);
            }
            printErr(L"usage: " + wstring(argv[0]) + L" usbrootdir playlist1 playlist2...");
            printErr(L"example:");
            printErr(wstring(argv[0]) + L" e:\\ EDM Rap Rock Pop");
            return 1;
        }        
        usbroot = argv[1];
        for (int i = 2; i < argc; ++i) {
            sync_playlists.insert(argv[i]);
        }

        throwIfFalse(::PathFileExists(usbroot.c_str()), usbroot + L" does not exist");

        throwIfFalse(::PathIsDirectory(usbroot.c_str()), usbroot + L" is not a directory");

        if (usbroot.length() > 0 && usbroot[usbroot.length() - 1] != L'\\')
            usbroot.push_back(L'\\');     

        ItunesPlaylists_t initunes;
        
        ItunesFiles_t itunesfiles;
        getPlaylists(sync_playlists, initunes, itunesfiles);
        
        DiskFiles_t ondisk;
        getFilesOnDisk(usbroot, ondisk);

        deleteFiles(usbroot, itunesfiles, ondisk);

        copyFiles(usbroot, itunesfiles, ondisk);        

        writePlaylists(usbroot, initunes);

    } catch (const std::bad_alloc&) {
        cerr << "memory allocation error" << endl;
        rval = 1;
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
        rval = 1;
    } catch (...) {
        cerr << "unknown exception" << endl;
        rval = 1;
    }

    return rval;
}
