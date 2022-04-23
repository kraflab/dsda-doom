### v0.12.1
- You can now play heretic

### v0.10.0
- The level time with tics now shows in the top left corner of the intermission screen
- New option: extended hud. This puts max totals, kill percent, and timers on top of the normal hud

### v0.9.0
- You can now rewind the game

### v0.8.4
- With demo overwrite off and the recording name matches an existing file, it picks the first unused name demo-01234.lmp
- Improved demo file cleanup (fix restart limit)

### v0.8.3
- Smart Totals renamed to Max Totals and "fixed" to show kill constraint for max

### v0.8.2
- Added options menu (in general)
- Added "strict mode" option, defaults to on. When on, tas settings are disabled while recording, unless you pass in the "-tas" parameter (always sr50, sr50 on turns, and game speed). This is a safeguard against accidentally leaving one of these settings on
- Added "cycle ghost colors" option, defaults to off

### v0.8.0
- Use the "restart current map" key while demo recording to perform an in-game restart

### v0.7.1
- Ghost files now support any number of players - a 4 player coop demo exports a 4 player ghost file
- Ghost color now cycles through the 4 player colors
- Fix for jittery ghosts at the end of the file

### v0.7.0
- Ghosts pause / fast-forward during movies to start each map together
- The -import_ghost command now accepts a list of files (-import_ghost ghost1 ghost2 ghost3 ...)

### v0.6.0
- Use -export_ghost xyz to create a ghost file (xyz.gst) - works during demo playback and recording
- Use -import_ghost xyz to import a ghost file (xyz.gst) - works during demo playback and recording

### v0.5.0
- Use -time_keys to show a split when you pick up a key
- Use -time_use to show a split when you press the use key
- Use -time_secrets to show a split when you find a secret
- Use -time_all to enable all the split options

### v0.4.0
- Added skill, nomonsters, respawn, fast, stroller, 100k, 100s, weapon_collector, tyson_weapons, turbo, and category to the analysis
- Added -track_100k option: show a notification when you reach 100% kills (as seen on intermission)
- Added sound effect for pacifist / 100k notifications

### v0.3.0
- Added missed_monsters and missed_secrets fields to the analysis

### v0.2.0
- Added reality and almost_reality fields to the analysis

### 0.1.0
- Use -analysis to set up an analysis.txt file (similar to levelstat.txt but for more general stuff, including pacifist verification). This will be expanded over time
- Use -track_pacifist to print a "Not pacifist!" message when you break the category rules
