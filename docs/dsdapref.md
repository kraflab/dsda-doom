## DSDAPREF Lump

The DSDAPREF lump contains dsda-doom-specific wad preferences.

### Specification

Each line of the DSDAPREF lump has a key and a value:

`key value`

The value can be ommitted (assumed to be `1`).

### Keys

- `prefer_software`
  - Shows an alert for users in opengl rendering mode
  - Use this if your wad relies on software rendering tricks
- `prefer_opengl`
  - Shows an alert for users in software rendering mode
  - Use this if your wad relies on opengl rendering tricks
- `use_mapinfo`
  - Turns on MAPINFO support
