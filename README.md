![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)
==============

syncplaylists
------
This is a Windows console-mode program I wrote to sync my iTunes playlists to a USB drive. 

You can download the executable from the [releases page](https://github.com/bailey27/syncplaylists/releases).  The executable is signed by the developer, Bailey Brown.  

There are no external dependencies.  However, you must have iTunes installed for syncplaylists to work, though. If you run syncplaylists, iTunes will automatically be launched if it isn't running already.  

You can build syncplaylists yourself if you want using the Community Edition of Microsoft Visual Studio. 

Whether or not syncplaylists does what you want, I think it would serve as reasonable example code for using the iTunes COM interface on Windows.

It is probably a good idea to let iTunes consolidate/organize your library before using syncplaylists (select file->library->organize library->consolidate files).

I have 2014 BMW F10, and I don't like to use my iPhone as an iPod with a cable because it skips songs sometimes.  I also don't like using bluetooth from the phone because I think I can hear the difference in sound quality vs. a hard-wired connection.

My car supports playing .m4a and .mp3 files from a USB flash drive.  It also supports .m3u playlists.

syncplaylists takes the path to the root of where you want the songs and playlists to go and a list of iTunes playlists you want to put on the device.

syncplaylists first *DELETES* any .m3u playlist files there, and it also *DELETES* any .m4a or .mp3 files that are there that aren't in the iTunes playlists you specified.

Then it copies any songs that aren't already there and writes .m3u play list files for each iTunes playlist you specified consisting of the name of a song on each line.  It copies only .m4a and .mp3 files.  It skips .m4p (DRM-protected) files because they won't work in the car.

The .m3u files are written using utf-8 character encoding and with CRLF (DOS) line endings.

syncplaylists always sorts the songs in the same order they are sorted in the play lists in iTunes.

Usage
----
Assuming your USB drive is mounted on drive e: and you want to sync the playlists named "EDM", "Rap", "Rock", and "Pop", then you would open a command prompt and run (assuming syncplaylists.exe is in your path)

```
syncplaylists.exe e:\ EDM Rap Rock Pop
```

Note: the names of playlists are case-sensitive.

Limitations
---
Because syncplaylists puts all the files int same directory, if there is a name collision between two different audio file names, then only one of them will end up being copied.  If this happens, then if you have iTunes organizing/consolidating your library, you can right-click on the song and select "song info" and change the name of the song a little or the track number, and the file will be renamed and the collision fixed.

Playlist folders have not been tested and probably won't work.

BUGS
----
When syncplaylists prints the names of the files being deleted or copied, certain non-English characters display as "?".  However, the file names are properly written in the .m3u play list files and the files are copied/deleted correctly.  This happens with some languages but not others.  Spanish and Czech characters display correctly, but Russian characters display as ? when printed to the console.

syncplaylists is outputting utf-8.  I tried calling SetConsoleOutputCP(CP_UTF8) but it appears to nave no effect on the problem.

However if you run this command before running syncplaylists

```

chcp 65001

```

which sets to code page to CP_UTF8 (65001), then Russian characters also display properly.

If that doesn't work, then you might need to change the console (cmd.exe) window's font to one that supports the language that isn't displaying properly.
