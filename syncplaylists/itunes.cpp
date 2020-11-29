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
#include <atlbase.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

#include "common.h"
#include "util.h"
#include "comhelper.h"

#include "iTunesCOMInterface.h"

namespace syncplaylists {
    namespace itunes {

        using namespace std;
        using namespace util;
        using namespace common;
        using namespace commhelper;

        void getPlaylists(const unordered_set<wstring>& sync_playlists,
            ItunesPlaylists_t& initunes,
            ItunesFiles_t& itunesfiles)
        {
            ComInitializer comInit; // constructor calls ::CoInitialize()      

            ComInterfaceWrapper<IiTunes> itunes;

            // note - CLSID_iTunesApp and IID_IiTunes are defined in iTunesCOMInterface_i.c
            auto hRes = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID*)&itunes.iface);

            throwIfFalse(hRes == S_OK, L"failed to connect to iTunes COM server");

            ComInterfaceWrapper<IITSourceCollection> iSources;

            hRes = itunes.iface->get_Sources(&iSources.iface);

            throwIfFalse(hRes == S_OK, L"failed to get sources");

            CComBSTR srclibname(L"Library");

            ComInterfaceWrapper<IITSource> library;

            hRes = iSources.iface->get_ItemByName(srclibname.m_str, &library.iface);

            throwIfFalse(hRes == S_OK, L"failed to get library");

            ComInterfaceWrapper<IITPlaylistCollection> playlists;

            hRes = library.iface->get_Playlists(&playlists.iface);

            throwIfFalse(hRes == S_OK, L"failed to get playlists");

            for (auto const& plname : sync_playlists) {
                CComBSTR bplname(plname.c_str());
                ComInterfaceWrapper<IITPlaylist> pl;
                hRes = playlists.iface->get_ItemByName(bplname.m_str, &pl.iface);
                throwIfFalse(hRes == S_OK, L"failed to get playist " + plname);

                ITPlaylistKind plkind;

                hRes = pl.iface->get_Kind(&plkind);
                throwIfFalse(hRes == S_OK, L"failed to get playist kind for " + plname);

                if (plkind != ITPlaylistKindUser) {
                    continue;
                }

                ComInterfaceWrapper<IITTrackCollection> tracks;
                hRes = pl.iface->get_Tracks(&tracks.iface);
                throwIfFalse(hRes == S_OK, L"failed to get tracks for " + plname);

                long count;

                hRes = tracks.iface->get_Count(&count);

                throwIfFalse(hRes == S_OK, L"failed to get count for " + plname);

                for (long i = 0; i < count; ++i) {
                    ComInterfaceWrapper<IITTrack> gt;
                    // indices are 1-based
                    hRes = tracks.iface->get_Item(i + 1, &gt.iface);
                    throwIfFalse(hRes == S_OK, L"failed to get item " + to_wstring(i) + L" in " + plname);                    
                    ITTrackKind tkind;
                    hRes = gt.iface->get_Kind(&tkind);                    
                    throwIfFalse(hRes == S_OK, L"failed to get track kind for item " + to_wstring(i) + L" in playist " + plname);
                    if (tkind != ITTrackKindFile) {
                        continue;
                    }
                    ComInterfaceWrapper<IITFileOrCDTrack> ft;
                    hRes = gt.iface->QueryInterface(IID_IITFileOrCDTrack, reinterpret_cast<void**>(&ft.iface));
                    throwIfFalse(hRes == S_OK, L"failed to get filetrack for item " + to_wstring(i) + L" in " + plname);                    

                    Song song;               

                    CComBSTR name;
                    hRes = ft.iface->get_Name(&name);

                    // we can proceed without the name if we don't get it
                    if (hRes == S_OK) {
                        song.name = name;
                    }

                    CComBSTR loc;
                    hRes = ft.iface->get_Location(&loc);
                    throwIfFalse(hRes == S_OK, L"failed to get location for song " + (song.name.length() > 0 ? song.name : L"at index " + to_wstring(i)) + L" in playlist " + plname);

                    song.filename = getFilename(loc.m_str);

                    if (::lstrcmpi(getExtension(song.filename).c_str(), L"m4p") == 0) {
                        printErr(L"skipping protected file " + song.filename);
                        continue;
                    }                                      

                    hRes = ft.iface->get_PlayOrderIndex(&song.order);
                    throwIfFalse(hRes == S_OK, L"unable to get play order index for song " + (song.name.length() > 0 ? song.name : song.filename) + L" in playlist " + plname);

                    itunesfiles[song.filename] = loc.m_str;

                    initunes[plname].emplace_back(song);
                }
            }
        }

    } // namespace itunes
} // namespace syncplaylists
