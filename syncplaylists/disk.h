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
	namespace disk {
		void getFilesOnDisk(const std::wstring& usbroot, std::unordered_set<std::wstring>& ondisk);

		void deleteFiles(const std::wstring& usbroot,
			const common::ItunesFiles_t& itunesfiles,
			const std::unordered_set<std::wstring>& ondisk);

		void copyFiles(const std::wstring& usbroot,
			const common::ItunesFiles_t& itunesfiles,
			const std::unordered_set<std::wstring>& ondisk);


		void writePlaylists(const std::wstring& usbroot,
			const std::unordered_map<std::wstring, std::vector<common::Song> >& initunes);

		// bare filenames
		typedef std::unordered_set<std::wstring> DiskFiles_t;
	} // namespace disk
} // namespace syncplaylists
