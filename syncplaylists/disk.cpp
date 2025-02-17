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

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>

#include "common.h"
#include "util.h"
#include "disk.h"

namespace syncplaylists {
	namespace disk {

        using namespace std;

        using namespace common;
        using namespace util;              

        static void asciiToLower(wstring& s)
        {
            // deliberately ignore locale
            auto len = s.length();
            for (size_t i = 0; i < len; ++i) {
                if (s[i] >= 'A' && s[i] <= 'Z')
                    s[i] += 'a' - 'A';
            }
        }

        static bool isInterestingFile(const wstring& filename)
        {
            const static unordered_set<wstring> deletable_exts = { L"m3u", L"mp3", L"m4a" };

            auto fileExt = getExtension(filename);

            if (fileExt.length() < 1)
                return false;

            asciiToLower(fileExt);

            return deletable_exts.find(fileExt) != deletable_exts.end();
        }

        static void writePlaylist(const wstring& usbroot,
            const wstring& plname,
            const vector<Song>& pl)
        {
            
            vector<const Song*> songs(pl.size());

            for (size_t i = 0; i < pl.size(); ++i) {
                songs[i] = &pl[i];
            }

            // we sort by playlist order         
            auto less_for_songs = [](const Song* a, const Song* b) -> bool {
                return a->order < b->order;                
            };

            sort(songs.begin(), songs.end(), less_for_songs);

            wstring plpath = usbroot + plname + L".m3u";

            auto open_file = [](const wstring& path) -> FILE* {
                FILE* f;
                if (::_wfopen_s(&f, path.c_str(), L"wb") == 0)
                    return f;
                else
                    return static_cast<FILE*>(nullptr);
            };

            auto close_file = [](FILE* fl) {if (fl) ::fclose(fl); };

            unique_ptr <FILE, decltype(close_file)>  fl(open_file(plpath), close_file);

            throwIfFalse(fl.get() != nullptr, L"unable to open " + plpath + L" for writing");

            string filename_utf8;

            for (auto song : songs) {               
                filename_utf8.clear();
                auto p = unicodeToUtf8(song->filename.c_str(), filename_utf8);
                throwIfFalse(p, L"cannot convert filename " + song->filename + L" to utf8");
                auto n = fwrite(p, 1, filename_utf8.length(), fl.get());
                throwIfFalse(n == filename_utf8.length(), L"did not write correct number of bytes to " + plpath);
                auto cw = fputc('\r', fl.get());
                throwIfFalse(cw != EOF, L"did not write correct number of bytes to " + plpath);
                cw = fputc('\n', fl.get());
                throwIfFalse(cw != EOF, L"did not write correct number of bytes to " + plpath);
            }

            printOut(L"wrote " + plpath);
        }

        // public functions
        void getFilesOnDisk(const wstring& usbroot, DiskFiles_t& ondisk)
        {
            WIN32_FIND_DATA fd;

            ::memset(&fd, 0, sizeof(fd));

            auto hFind = ::FindFirstFile((usbroot + L"*").c_str(), &fd);

            throwIfFalse(hFind != INVALID_HANDLE_VALUE, L"error finding files in " + usbroot);

            while (hFind != INVALID_HANDLE_VALUE) {

                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                    if (::wcscmp(fd.cFileName, L".") != 0 && ::wcscmp(fd.cFileName, L"..") != 0) {
                        printOut(wstring(L"ignoring directory ") + fd.cFileName);
                    }
                } else if (!isInterestingFile(fd.cFileName)) {
                    printOut(wstring(L"ignoring file ") + fd.cFileName);
                } else {
                    ondisk.insert(fd.cFileName);
                }

                if (!::FindNextFile(hFind, &fd))
                    break;
            }

            // if FindNextFile returns FALSE because it's finished then it sets LastError to ERROR_NO_MORE_FILES
            // capture LastError before doing antyhing else
            auto LastErr = ::GetLastError();

            if (hFind != INVALID_HANDLE_VALUE) {
                ::FindClose(hFind);
            }

            throwIfFalse(LastErr == ERROR_NO_MORE_FILES, L"FindNextFile on " + usbroot + L" returned error " + to_wstring(LastErr));
        }

        void deleteFiles(const wstring& usbroot,
            const ItunesFiles_t& itunesfiles,
            const DiskFiles_t& ondisk)
        {
            for (auto const& it : ondisk) {
                wstring path = usbroot + it;
                if (itunesfiles.find(it) == itunesfiles.end()) {
                    auto delRes = ::DeleteFile(path.c_str());
                    if (delRes) {
                        printOut(L"deleted " + path);
                    }
                    throwIfFalse(delRes, L"failed to delete " + path);
                }
            }
        }

        void copyFiles(const wstring& usbroot,
            const ItunesFiles_t& itunesfiles,
            const DiskFiles_t& ondisk)
        {
            for (auto const& it : itunesfiles) {
                wstring dst = usbroot + it.first;
                bool shouldCopy = false;

                // Check if the file is missing in the destination
                auto found = ondisk.find(it.first);
                if (found == ondisk.end()) {
                    shouldCopy = true;
                }
                else {
                    // File exists; check file sizes
                    LARGE_INTEGER srcSize, dstSize;
                    HANDLE hSrc = CreateFile(it.second.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    HANDLE hDst = CreateFile(dst.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                    if (hSrc != INVALID_HANDLE_VALUE && hDst != INVALID_HANDLE_VALUE) {
                        if (GetFileSizeEx(hSrc, &srcSize) && GetFileSizeEx(hDst, &dstSize)) {
                            if (srcSize.QuadPart != dstSize.QuadPart) {
                                shouldCopy = true;
                            }
                        }
                    }

                    if (hSrc != INVALID_HANDLE_VALUE) CloseHandle(hSrc);
                    if (hDst != INVALID_HANDLE_VALUE) CloseHandle(hDst);
                }

                // Copy the file if needed
                if (shouldCopy) {
                    auto cpRes = ::CopyFile(it.second.c_str(), dst.c_str(), FALSE);
                    if (cpRes) {
                        printOut(L"copied " + dst);
                    }
                    throwIfFalse(cpRes, L"failed to copy " + dst);
                }
            }
        }

        void writePlaylists(const wstring& usbroot,
            const ItunesPlaylists_t& initunes)
        {
            for (auto const& it : initunes) {
                writePlaylist(usbroot, it.first, it.second);
            }
        }

	} // namespace disk
} // namespace syncplaylists
