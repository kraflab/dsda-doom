## SNDINFO

SNDINFO is a lump that allows authors to configure various advanced sound features. In DSDA-Doom, this lump is currently only used to define ambient sound effects.

## Usage

Simply include the SNDINFO lump in your wad and dsda-doom will load it.

### Specification

In order to reference sound effects, you need to define them:

`<sound_name> = <lump_name>`

Here `sound_name` is the identifier you use when referring to the sound and `lump_name` is the name of the sound effect lump. DSDA-Doom does not have any predefined sounds.

Once you have defined a sound effect, you can define ambient sound effects like so:

`$ambient <index> <sound_name> [type] <mode> <volume>`

The `index` is used when placing ambient sounds in a map. The `sound_name` is described above. You **can** define multiple ambient sounds that use the same `sound_name` with different properties. The optional `type` is either `point <attenuation>` or `world` (default). If using a `point` sound, you can define the `attenuation` rate, which determines how fast the volume drops off with distance (default `1.0`). The `mode` can be `continuous` (looping), `periodic <seconds>`, or `random <min_seconds> <max_seconds>`. Finally, the `volume` sets the volume, ranging from `0.0` to `1.0` (default). All numerical arguments are floating points.

Example:

```
wolf = k_wolf
waterfall = k_wfall
bell = k_bell

$ambient 1 wolf world random 30.0 60.0 0.5
$ambient 2 waterfall point 2.0 continuous 1.0
$ambient 3 bell periodic 60.0 1.0
```

In this file, we define three sounds: a wolf, a waterfall, and a bell. Then we define 3 ambient sounds:

1) A wolf that randomly occurs every 30 to 60 seconds at half volume and can be heard everywhere
2) A waterfall that is located at a point, plays continuously, and decays twice as fast with distance as normal sounds
3) A bell that rings every 60 seconds

### Placing ambient sounds in maps

You can place an ambient sound in a map using DoomEdNum 14001 to 14064, where the associated index ranges from 1 to 64. Optionally, you can use DoomEdNum 14065 and set the first special argument to the ambient sound index.
