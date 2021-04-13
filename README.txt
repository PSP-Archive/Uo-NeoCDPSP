Changes since version 0.6.2 :

 - Fixed crash in breakers : this is once again related to the upload area,
   so I can just hope it won't break other games
 - Try harder to recognise game files from ipl.txt : some originals have
   weird filenames in the ipl.txt (a double dragon version is in this case).
   Now if I can't find the file type in the extension, I look for it in
   the whole filename, which seems to work for this double dragon version,
   at least.
 - Fixed sound effects : they are now much clearer, and in real stereo.
   The difference is especially noticeable in bust a move.
 - Fixed a last possible corruption of the memcard.bin file if quiting with
   the home key. Now it should be safe to use the home key to quit at any
   time, and there should be no way left to corrupt any file (mp3 or other).
   If you use fw 2.0, be sure to use the Exit command from the gui though,
   or do otherwise at your own risk (I am not testing neocdpsp with fw 2.0)
 - The sound effects do not stop anymore after 3 or 4 games (finally !)
 - Added some game messages at the bottom of the screen (now you know
   when an mp3 track is starting to play, if it didn't find it, if there
   was an error decoding it, and so on...)
 - In previous version, if an mp3 had some id3 tags it couldn't be played
   in neocdpsp. This is fixed now, sorry for those who encountered this
   problem.
 - Now you can also place your mp3s in PSP/MUSIC. Example :
   say you have a game in a directory mslug. The you can put its mp3s
   as previously in mslug/neocd.mp3/ but now you can also put them in
   /PSP/MUSIC/mslug/ if you prefer. After all these are mp3s, so you should
   be able to play them from the psp interface if you want to !
   The directory in music where you put the mp3 must be named exactly like
   the directory of the game for neocdpsp.
 - The mp3 decoder is a little more resistant to crashes, that is if you have
   an mp3 file at 0 or 1 byte after you have filled your memory card, it
   won't crash anymore. I don't say it will not every crash again though,
   there are probably files which can make it crash !
 - Fixed sound and music in top players golf
 - Fixed Art of Fighting 3 freezing at start of gameplay
 - The loading of zipped games is even faster than in 0.6.2 !
 - The ingame sound effects of samurai spirits rpg work now, the game is
   still very long to load, but it was already long on the original hardware
   and is now limited by the access speed of the memory card.
 - Now the memory card is writen only when you quit the game, not everytime
   the game tries to write to it.
 - Added "Select lock" option to the gui. When enabled the controls become:
   Select + LTrigger instead of LTrigger for the gui
   Select + RTrigger instead of RTrigger for snapshots
   and Select + Cross instead of Select alone (pause in most games).
   This for those who press the triggers by accident !

The source should be available at the same place as this.

Here is the original readme file from version 0.5 for convinience :

 -------------
/NeoCDPSP 0.5/
-------------

+++++++++++++++++++++++++++++
[INTRO]
--------------------------->

NeoCDPSP is based on :

Neo4ALL, an "Alternative Open Source NEOGEO/CD emulator for the Dreamcast console by Chui & Fox68k".
--> http://chui.dcemu.co.uk/neo4all.shtml

Ruka's psp emulators, for valuable sourcecode (learned a lot from it!).
--> http://rukapsp.hp.infoseek.co.jp/ 

Previous neocd psp emulator, NeoCD/PSP.
--> http://psp-dev.org/pukiwiki/index.php?NEOGEO%20CD

unofficial PSPSDK.
--> http://www.ps2dev.org

GFX from Pochi (psp background, icon, loading)
-->http://pochistyle.pspwire.net/

GFX from SNK (file selector & menu background)

+++++++++++++++++++++++++++++
[FEATURES]
--------------------------->
- neogeo cd emulation
- 2 gfx engine : software of hardware (using psp's gu)
- autofire
- memory card emulation
- zipped/unzipped game
- sound support
- music support (with mp3 tracks, using libmad)
- mutiples rendering mode with hardware stretching (thx to pspsdk & chp great work!)
- 222/266/333 Mhz

+++++++++++++++++++++++++++++
[INSTALL]
--------------------------->

/PSP/GAME/NEOCDPSP/
(or any others directory since v0.5)
                  + EBOOT.PBP
                  + STARTUP.BIN
                  + loading.bmp
                  + logo.bmp
                  + NEOCD.BIN (BIOS)
           
Games can be zipped or unzipped.
Music in mp3 format have to be in a "neocd.mp3" subfolder.

Example :

/PSP/GAME/NEOCDPSP/MetalSLug/
                            + mslug.zip
                            + neocd.mp3/
                                       + Metal Slug - Track 02.mp3
                                       + Metal Slug - Track 03.mp3           
                                       + Metal Slug - Track 04.mp3
                                       ....
                                       + Metal Slug - Track 20.mp3

You can uncompress the zip file for faster loading time.
MP3 Tracks have to finish by the "tracker number.mp3" 

=> "mslug-02.mp3" is good
=> "02-mslug.mp3" is wrong

+++++++++++++++++++++++++++++
[PLAY]
--------------------------->

PSP           NEOGEO
pad,analog    pad
CROSS         A
CIRCLE        B
SQUARE        C
TRIANGLE      D
START         START
SELECT        SELECT
LEFT TRIGGER  MENU
RIGHT TRIGGER SNAPSHOT

+++++++++++++++++++++++++++++
[HISTORY]

-
0.5
* Music (mp3) playback bug in pause/play fixed (Last Resort seems ok now).
* Rendering fixed (sprite disappearing)
* New render using psp hardware 
* Improved GUI
* New feature : autofire for A,B,C,D neogeo buttons
* Changed memory io and z80 emulation with NeoDC 2.3 sources (music in more games)
* New GFX from great pochi! http://pochiicon.hp.infoseek.co.jp/
* Screenshot (press R trigger), saved in uncompressed 24bits BMP (480x272)
* Directory independant, you only have to put the whole files & bios in the same dir
(no "/PSP/GAME/NEOCDPSP" directory only anymore)
-
0.1 - 2005/07/07 (first release)
-

+++++++++++++++++++++++++++++
[COMING NEXT]

* load/save state
* 

+++++++++++++++++++++++++++++
[KNOWN BUGS]

* emulation isn't perfect (lack of technical documentation for the neogeo cd)
-> so no sound/no music in some games
-> sometimes music is stopped when putting game in pause (pressing select/start) and 
not resumed when resuming game.
* some games won't work at all

* If you want this to change, you'll have to find me neogeo cd technical documentations
or an emulator which does the job better with sourcecode available ;-)

+++++++++++++++++++++++++++++
[EXIT]
--------------------------->

Have fun!

------------------>
http://yoyofr.fr.st
----->

yoyofr
 -------------
/NeoCDPSP 0.5/
-------------

+++++++++++++++++++++++++++++
[INTRO]
--------------------------->

NeoCDPSP is based on :

Neo4ALL, an "Alternative Open Source NEOGEO/CD emulator for the Dreamcast console by Chui & Fox68k".
--> http://chui.dcemu.co.uk/neo4all.shtml

Ruka's psp emulators, for valuable sourcecode (learned a lot from it!).
--> http://rukapsp.hp.infoseek.co.jp/ 

Previous neocd psp emulator, NeoCD/PSP.
--> http://psp-dev.org/pukiwiki/index.php?NEOGEO%20CD

unofficial PSPSDK.
--> http://www.ps2dev.org

GFX from Pochi (psp background, icon, loading)
-->http://pochistyle.pspwire.net/

GFX from SNK (file selector & menu background)

+++++++++++++++++++++++++++++
[FEATURES]
--------------------------->
- neogeo cd emulation
- 2 gfx engine : software of hardware (using psp's gu)
- autofire
- memory card emulation
- zipped/unzipped game
- sound support
- music support (with mp3 tracks, using libmad)
- mutiples rendering mode with hardware stretching (thx to pspsdk & chp great work!)
- 222/266/333 Mhz

+++++++++++++++++++++++++++++
[INSTALL]
--------------------------->

/PSP/GAME/NEOCDPSP/
(or any others directory since v0.5)
                  + EBOOT.PBP
                  + STARTUP.BIN
                  + loading.bmp
                  + logo.bmp
                  + NEOCD.BIN (BIOS)
           
Games can be zipped or unzipped.
Music in mp3 format have to be in a "neocd.mp3" subfolder.

Example :

/PSP/GAME/NEOCDPSP/MetalSLug/
                            + mslug.zip
                            + neocd.mp3/
                                       + Metal Slug - Track 02.mp3
                                       + Metal Slug - Track 03.mp3           
                                       + Metal Slug - Track 04.mp3
                                       ....
                                       + Metal Slug - Track 20.mp3

You can uncompress the zip file for faster loading time.
MP3 Tracks have to finish by the "tracker number.mp3" 

=> "mslug-02.mp3" is good
=> "02-mslug.mp3" is wrong

+++++++++++++++++++++++++++++
[PLAY]
--------------------------->

PSP           NEOGEO
pad,analog    pad
CROSS         A
CIRCLE        B
SQUARE        C
TRIANGLE      D
START         START
SELECT        SELECT
LEFT TRIGGER  MENU
RIGHT TRIGGER SNAPSHOT

+++++++++++++++++++++++++++++
[HISTORY]

-
0.5
* Music (mp3) playback bug in pause/play fixed (Last Resort seems ok now).
* Rendering fixed (sprite disappearing)
* New render using psp hardware 
* Improved GUI
* New feature : autofire for A,B,C,D neogeo buttons
* Changed memory io and z80 emulation with NeoDC 2.3 sources (music in more games)
* New GFX from great pochi! http://pochiicon.hp.infoseek.co.jp/
* Screenshot (press R trigger), saved in uncompressed 24bits BMP (480x272)
* Directory independant, you only have to put the whole files & bios in the same dir
(no "/PSP/GAME/NEOCDPSP" directory only anymore)
-
0.1 - 2005/07/07 (first release)
-

+++++++++++++++++++++++++++++
[COMING NEXT]

* load/save state
* 

+++++++++++++++++++++++++++++
[KNOWN BUGS]

* emulation isn't perfect (lack of technical documentation for the neogeo cd)
-> so no sound/no music in some games
-> sometimes music is stopped when putting game in pause (pressing select/start) and 
not resumed when resuming game.
* some games won't work at all

* If you want this to change, you'll have to find me neogeo cd technical documentations
or an emulator which does the job better with sourcecode available ;-)

+++++++++++++++++++++++++++++
[EXIT]
--------------------------->

Have fun!

------------------>
http://yoyofr.fr.st
----->

yoyofr
