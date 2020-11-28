#pragma once
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

namespace syncplaylists {
    namespace commhelper {        

        struct ComInitializer {
            bool initialized;
            ComInitializer()
            {
                initialized = ::CoInitialize(nullptr) == S_OK;
                util::throwIfFalse(initialized, L"unable to initialize COM");
            }
            ~ComInitializer()
            {
                if (initialized)
                    ::CoUninitialize();
            }
            // disallow copying
            ComInitializer(ComInitializer const&) = delete;
            void operator=(ComInitializer const&) = delete;
        };

        template <typename T>
        struct ComInterfaceWrapper {
            T* iface;
            ComInterfaceWrapper() : iface(nullptr) {}
            ~ComInterfaceWrapper()
            {
                if (iface)
                    iface->Release();
            };
            // disallow copying
            ComInterfaceWrapper(ComInterfaceWrapper const&) = delete;
            void operator=(ComInterfaceWrapper const&) = delete;
        };
       
    } // namespace syncplaylists
} // namespace commhelper
