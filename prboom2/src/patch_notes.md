### v0.19.0

#### MBF21 - Modder's Best Friend - A New Standard
- See separate file for specification
- Changed default complevel to 21 (mbf21)

#### Save Organization
- New option to let dsda-doom organize your saves
  - Save files will be separated based on the wad files you use
  - The location will be old_place/save_files/iwad/pwad/...
  - Directories are created automatically
- There are now pages in the save / load screens, giving you more file slots

#### Command Display
- New option to display command history
- Disabled while strict mode is active
- Can be used for playback analysis, tasing, practice, etc

#### Coordinate Display
- New option to display coordinates / velocity
- Disabled while strict mode is active
- Position display uses doom units
  - Doom uses fractions instead of floating points for position variables
  - For example, the coordinate 3.7 doesn't exist. The closest position is 3 + 45875 / 65536
  - This will display as 3.45875 so that you have the exact location without approximations
- Velocity display is color-coded based on speed thresholds:
  - speed achievable without straferunning
  - speed achievable with sr40
  - speed achievable with sr50
  - speed beyond sr50 range
- Useful for figuring out setups, tasing, practice, etc

#### Heretic Cheats
- Added heretic cheat codes
- Can use doom cheats in heretic and vice versa
- The only new code relevant for doom is "ponce" (reset health)
- Input bindings for tome, chicken, and health reset cheats

#### Command Console
- Added a console where you can enter various commands
- Set up a key binding to access
- Command list: WIP

#### Miscellaneous
- MBF sky transfers work in all complevels (pr+)
- Map coordinates are protected with strict mode
- Crosshair target color / lock are protected with strict mode
- Updated to umapinfo rev 1.6 (pr+)
- Changed demo overwrite default to off
- Removed deprecated laggy sleep option
- Removed various unwelcome / secret configuration options
- Removed option to turn demo footer off
- Added mbf OPTIONS lump support
- Added default map name fallback to automap
- Added comp_respawnfix alias for cross-port OPTIONS support (EE)
- Added option to hide the heretic hud horns
- Added notification when toggling vertmouse
- Added notification when toggling mouselook
- Added notification when changing game speed
- Added strict mode toggle input binding
- Added option to disable sound cutoffs (pr+)
- Added Fireb3rt advanced hud layout option (pr+)
- Fixed missing small screen border for heretic
- Fixed zoomed in heretic weapon heights
- Fixed heretic scrolling floors also scrolling the ceiling
- Fixed dsda-doom save game header
- Fixed a bug where mouselook data was not reset when using demo restart
- Fixed an issue with wad lump loading - small performance improvement
- Fixed an issue with sound effects causing lag spikes for some users
- Fixed msvc build (xaser)
- Fixed macos launcher iwad directory mismatch (PBeGood4)
- Fixed umapinfo custom episode select for doom2 (pr+)
- Fixed shifting automap markers (pr+)
- Fixed flood hom in plutonia map 11 in opengl (pr+)
- Fixed sky walls as seen in e4m6 in opengl (pr+)
- Fixed transparent sprites in opengl (pr+)
- Fixed non-standard sky scaling in opengl (pr+)
- Fixed issue with windows dpi virtualization (pr+)
- Fixed umapinfo musinfo (pr+)
- Fixed umapinfo endbunny / endgame fields (pr+)
