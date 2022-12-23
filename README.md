# dsda-doom v0.24.3
This is a fork of prboom+ with many new features, including:
- Heretic, Hexen, MBF21, and Doom-in-Hexen support
- In-game console and scripting
- Full controller support
- Palette-based lightmode for opengl
- Debugging features for testing
- Strict mode for speedrunning
- Various quality of life improvements
- Advanced tools for TASing
- Rewind

Download windows releases [here](https://drive.google.com/drive/folders/1KMU1dY0HZrY5h2EyPzxxXuyH8DunAJV_?usp=sharing).

### Patch Notes
- [v0.24](./patch_notes/v0.24.md)
- [v0.23](./patch_notes/v0.23.md)
- [v0.22](./patch_notes/v0.22.md)
- [v0.21](./patch_notes/v0.21.md)
- [v0.20](./patch_notes/v0.20.md)
- [v0.19](./patch_notes/v0.19.md)

### Launcher
There is a dedicated launcher for this port available [here](https://github.com/Pedro-Beirao/dsda-launcher) by PBeGood4.

### Doom-in-Hexen Support
- [Full details](./docs/doom_in_hexen.md)

### Hexen Support
- DSDA-Doom includes demo-compatible support for hexen.
  - Use -iwad HEXEN.WAD (-file HEXDD.WAD for the expansion)
    - Or drag wads onto the exe
  - You can force hexen engine behaviour with `-hexen` (shouldn't be necessary)
- Don't need to supply complevel (hexen is complevel 0 by necessity)
- Known issues
  - Setting the "Status Bar and Menu Appearance" option to "not adjusted" will have no effect for hexen (it will default instead to "Doom format")
  - The "Apply multisampling" automap option is disabled for hexen
  - Automap colors are not configurable for hexen
  - Some of the more advanced features are not implemented for hexen yet, and using them may cause crashes or other odd behaviour.
  - Some menus extend over the hud.
  - Monster counter doesn't work as expected, due to cluster format (ex hud / levelstat)
  - Hexen-style skies aren't implemented yet (layering, etc)
  - The ALTSHADOW thing flag isn't affecting the rendering
  - Dynamic fade palettes aren't being used
  - The yellow message variant isn't implemented

### Heretic Support
- DSDA-Doom includes demo-compatible support for heretic (all the demos stored on dsda are in sync).
- Heretic game logic should be set automatically if you use `HERETIC.WAD` as the iwad. If it doesn't work, please use the `-heretic` commandline option. This flips a switch in the engine that determines all the core game data.
- You do not need to (and can't) set the complevel when playing heretic. It is implicitly set to `0`, since heretic is based on doom v1.2.
- Known issues
  - Setting the "Status Bar and Menu Appearance" option to "not adjusted" will have no effect for heretic (it will default instead to "Doom format").
  - The "Apply multisampling" automap option is disabled for heretic.
  - Automap colors are not configurable for heretic.
  - Some of the more advanced features are not implemented for heretic yet, and using them may cause crashes or other odd behaviour.
  - Dehacked support for heretic isn't implemented yet.
  - Some menus extend over the hud.

### Credits
- The DSDA-Doom icon was designed by Mal (129thVisplane). Thanks!
