### Tower of Trials (v3.14 r59) by Jdaster64

### Overview
**Tower of Trials** is a Rogue-like mod of Paper Mario: TTYD by Jdaster64.
In it, Mario storms the newly-renovated Hooktail Castle, fighting through a 
battery of enemy encounters and building up an arsenal of party members,
moves, badges and other rewards!  Once you've finished a run, you can spend
your winnings on items and cosmetic upgrades in Petalburg, or do more runs
to work toward 100% completion of the Journal, including dozens of achievements!

Nearly all random elements of runs are seeded, so once you unlock the ability
to set a seed, you can race other players on a level playing field!

For a detailed overview of Tower of Trials' unique features, see
[the PDF companion guide](https://drive.google.com/file/d/1qrbCXvisoQLUZn7wvW3QxfmONejAEHKG/view?usp=sharing).

If you're interested, check out [this page](https://bit.ly/jdaster64-mario-rpg-mods)
for information on some of my other Mario RPG mods.

### Setup
Before attempting to install the mod, make sure you have **an unmodified image
of the North American (U) version of TTYD** for the GameCube
(md5 of **db9a997a617ee03bbc32336d6945ec02**).

**Tower of Trials _will not work_ on any other version of the game,
and support is not guaranteed with other mods, custom textures, etc.**

After verifying you have a working copy of the game, simply run the appropriate
patcher executable for your platform from the
["Releases" page](https://github.com/jdaster64/ttyd-tot/releases),
provide it the unmodified TTYD .iso image, and it will generate a patched copy
of the game, which you can simply run, without any loader codes or such.

Tower of Trials uses a separate save data file from the original TTYD, so make
sure whatever memory card you're using has space for an additional 17 blocks.

### Known Issues
*   Loading a file with a run already in progress will cause the next floor's
    map to look like the bottom floors of the tower, regardless of progress.
    This will likely not be fixed, as it is only a visual issue and will correct
    itself on the following floor, and making the map information propagate
    through TTYD's various loading-zone object systems is tricky to do
    without breaking them.

### Credits
*   **PistonMiner** - For the initial REL framework and related tools,
    Blender export script, and ttydasm, which was immensely useful as
    reference for vanilla TTYD event scripts; all of these can be found on the 
    [ttyd-tools repository](https://github.com/PistonMiner/ttyd-tools).
*   **Zephiles** - For the 
    [TTYD practice codes](https://github.com/Zephiles/TTYD-Practice-Codes) 
    (both for ease of testing, and for implementation reference), 
    contributions towards the REL framework (in particular, methods for 
    loading the REL).
*   **Diagamma** for the Melvin patcher and help putting together the
    auto-bootloader assembly script.
*   **Seeky** for adding REL linking support to elf2rel and writing an
    EVT disassembler that outputs in C macro format.
*   **Peech** for streamlining the process of exporting maps to/from Blender.
*   **Peardian** for the files used as a base for the new maps.
*   **MuzYoshi** for the title screen background image.
*   Everyone that contributed to reverse-engineering / documentation work
    on TTYD, including PistonMiner, NWPlayer123, Zephiles, SolidifiedGaming,
    Jasper, Isocitration, Malleo and others.

### Special Thanks
*   Fatguy703, Gibstack, Kappy and others for pre-release playtesting and feedback.
*   Kaze Emanuar and the F3 editing team for the F3 trailer features.

