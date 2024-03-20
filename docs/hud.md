## HUD Configuration

HUD configurations are stored in the DSDAHUD lump. These defaults can be changed by replacing the lump or specifying a hud config file with `-hud filename`. There are separate configurations for each game (doom, heretic, and hexen), with or without the status bar.

### Specification

A HUD configuration starts with the environment: `game variant`
- The `game` is `doom`, `heretic`, or `hexen`
- The `variant` options are:
  - `ex` (with status bar and extended hud on)
  - `off` (with status bar and extended hud off)
  - `full` (without status bar)
  - `map` (on top of the automap)

The configuration then consists of a series of components to display: `name x y alignment [args]`
- The `name` specifies which component to display
- The `x` and `y` fields set the location of the component
  - `y` values are automatically adjusted for different font heights
- The `alignment` controls how the position is translated in different screen sizes
  - `top`
  - `top_left`
    - This alignment accounts for the size of the message area
  - `top_right`
  - `bottom`
  - `bottom_left`
  - `bottom_right`
  - `left`
  - `right`
  - `none`
- The `args` are a series of optional parameters (see specific components for more info)
- For convenience, bottom-aligned `y` values are the distance from the bottom of the screen (`full`) or top of the status bar (`ex` and `off`)

This example configures the extended hud for doom with just the stat totals and time in the bottom left (above the status bar):
```
doom ex
stat_totals 2 8 bottom_left
composite_time 2 16 bottom_left
```

Finally, there is a positioning helper: `add_offset y alignment`
- This compensates for mixed stretching between the message font and the extended hud font
- Set `y` as the number of message font component lines at the edge of the given alignment

You can find the current default configuration [here](../prboom2/data/lumps/dsdahud.lmp).

### Components

Unless otherwise specified, argument values are integers. For toggles, a 1 means on and a 0 means off. For example, `stat_totals 2 8 bottom_left 1 0 1` would turn off items but keep kills and secrets enabled.

- `stat_totals`: shows the kills / secrets / items on the current map
  - Supports 6 arguments: `show_kills show_items show_secrets vertical show_labels hide_totals`
  - `show_kills`: shows kills in the component
  - `show_items`: shows items in the component
  - `show_secrets`: shows secrets in the component
  - `vertical`: displays the stats vertically rather than horizontally
  - `show_labels`: shows the "K" "I" "S" labels
  - `hide_totals`: hides the total counts until they are reached
- `composite_time`: shows the current level time and the total time
  - Supports 1 argument: `show_label`
  - `show_label`: shows the "time" label
- `keys`: shows the acquired keys
  - Supports 1 argument: `horizontal`
  - `horizontal`: displays the component horizontally rather than vertically
- `ammo_text`: shows the weapons and ammo as the status bar does
  - Supports 1 argument: `show_names`
  - `show_names`: shows ammo names in the component
- `weapon_text`: shows the acquired weapons (color-coded for berserk).
  - Supports 2 argument: `grid show_label`
  - `grid`: displays the weapons in a 3x3 grid rather than horizontally
  - `show_label`: shows the "wpn" label
- `ready_ammo_text`: shows the ammo for the current weapon
- `big_ammo`: shows the ammo for the current weapon in the status bar font
- `armor_text`: shows the player armor (color-coded)
- `big_armor`: shows the player armor (color-coded) in the status bar font with the armor sprite
- `big_armor_text`: shows the player armor (color-coded) in the status bar font
- `health_text`: shows the player health (color-coded)
- `big_health`: shows the player health (color-coded) in the status bar font with the health sprite
- `big_health_text`: shows the player health (color-coded) in the status bar font
- `big_artifact`: shows the current artifact as seen on the status bar
- `fps`: shows the current fps
- `attempts`: shows the current and total demo attempts
- `render_stats`: shows various render stats (`idrate`)
- `speed_text`: shows the game clock rate
  - Supports 1 argument: `show_label`
  - `show_label`: shows the "speed" label
- `command_display`: shows the history of player commands (demo or otherwise)
- `coordinate_display`: shows various coordinate and velocity data
- `event_split`: shows the time of an event tracked by the `-time_*` arguments
- `level_splits`: shows the splits for the level time and the total time (intermission screen)
- `line_display`: shows the last lines the player activated
- `tracker`: shows the active trackers (they stack *vertically*)
- `local_time`: shows the local time
- `minimap`: shows the minimap
  - Supports 3 arguments: `width height scale`
  - `width`: width of the component
  - `height`: height of the component
  - `scale`: width of the component in map units
- `color_test`: shows the hud fonts in different color modes
- `free_text`: shows arbitrary text
  - Update the text in the console with `free_text.update <text>`
  - Clear the text in the console with `free_text.clear`
  - Use `\n` to create a new line
  - Use `\cXY` to change to color `XY`
- `map_totals`: shows the kills / secrets / items on the current map
  - Uses the message font with word labels
  - Supports 4 arguments: `show_kills show_items show_secrets hide_totals`
  - `show_kills`: shows kills in the component
  - `show_items`: shows items in the component
  - `show_secrets`: shows secrets in the component
  - `hide_totals`: hides the total counts until they are reached
- `map_time`: shows the level / total time
  - Uses the message font
- `map_coordinates`: shows the player's position
  - Uses the message font
- `map_title`: shows the current map's title
  - Uses the message font
- `message`: shows the current player message
  - Uses the message font
  - Supports 1 argument: `center`
  - `center`: centers the component horizontally
- `secret_message`: shows the secret revealed message
  - Uses the message font
  - Supports 1 argument: `center`
  - `center`: centers the component horizontally
