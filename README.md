![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)
==============

syncplaylists
------
This is a Windows program I wrote to sync my iTunes playlists to a USB drive. 

You can download the executable from the [releases page](https://github.com/bailey27/syncplaylists/releases).  The executable is signed by the developer, Bailey Brown.  

There are no external dependencies.  However, you must have iTunes installed for syncplaylists to work, though. If you run syncplaylists, iTunes will automatically be launched if it isn't running already.  

You can build it yourself if you want using the Community Edition of Microsoft Visual Studio. 

Whether or not it does what you want, I think it would serve as reasonable example code for using the iTunes COM interface on Windows.

I have 2014 BMW F10, and I don't like to use my iPhone as an iPod with a cable because it skips songs sometimes.  I also don't like using bluetooth from the phone because I think I can hear the difference in sound quality vs. a hard-wired connection.

My car supports playing .m4a and .mp3 files from a USB flash drive.  It also supports .m3u playlists.

syncplaylists takes the path to the root of where you want the songs and playlists to go and a list of iTunes playlists you want to put on the device.

syncplaylists first *DELETES* any .m3u playlist files there, and it also *DELETES* any .m4a or .mp3 files that are there that aren't in the iTunes playlists you specified.

Then it copies any songs that aren't already there and writes .m3u playlists files for each iTunes playlist you specified consisting of the name of a song on each line.  It copies only .m4a and .mp3 files.  It skips .m4p (DRM-protected) files because they won't work in the car.

syncplaylists always sorts the songs alphabetically by name and ignores leading "A" and leading "The" as well as ignoring track numbers.

I found that iTunes does not return the songs in the same order they are sorted in the playlists in iTunes.  I want the songs alphabetically sorted anyway.

Usage
----
Assuming your USB drive is mounted on drive e: and you want to sync the playlists named "EDM", "Rap", "Rock", and "Pop", then you would open a command prompt and run (assuming syncplaylists.exe is in your path)

```
syncplaylists.exe e:\ EDM Rap Rock Pop
```

Note: the names of playlists are case sensitive.

BUGS
----
When syncplaylists prints the names of the files being deleted or copied, certain non-English characters display as "?".  However, the filenames are properly written in the .m3u files and the files are copied/deleted correctly.  This happens with some languages but not others.  Spanish and Czech characters display correctly, but Russian characters display as ? when printed to the console.

