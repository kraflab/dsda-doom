### v0.19.0

#### MBF21 - Modder's Best Friend - A New Standard
- See separate file for specification
- Changed default complevel to 21 (mbf21)

#### Save Organization
- New option to let dsda-doom organize your saves
  - Save files will be separated based on the wad files you use
  - The location will be old_place/save_files/iwad/pwad/...\
  - Directories are created automatically
- There are now pages in the save / load screens, giving you more file slots

#### Command Display
- New option to display command history
- Disabled while strict mode is active
- Can be used for playback analysis, tasing, practice, etc

#### Miscellaneous
- Remove deprecated laggy sleep option
- Remove various unwelcome / secret configuration options
- Add mbf OPTIONS lump support
- Add default map name fallback to automap
- Add comp_respawnfix alias for cross-port OPTIONS support (EE)
- Add option to hide the heretic hud horns
- Add notification when toggling vertmouse
- Add notification when toggling mouselook
- Fix missing small screen border for heretic
- Fix zoomed in heretic weapon heights
- Fix heretic scrolling floors also scrolling the ceiling
- Fix dsda-doom save game header
- Fix a bug where mouselook data was not reset when using demo restart
- Fix msvc build (xaser)
- Fix macos launcher iwad directory mismatch (PBeGood4)
