# DSDHacked

DSDA-Doom supports "unlimited" states, sprites, sounds, and things in dehacked (until you run out of memory / reach 2^31 - 1). This is an extension of mbf21, which vastly increased the demand for raised limits in dehacked. This document is a specification of the behaviour.

Use `Doom version = 2021` in your dehacked file to signal that you are using dsdhacked indices. This allows ports to adapt your changes to their specific internal tables. Otherwise your file may be interpreted differently in different ports.

### Defaults

When you define a new index, the game allocates new entities up to that value. The data is filled with default values, listed here. Anything not specifically mentioned is set to zero / null.

#### State

| Field      | Value                   |
|------------|-------------------------|
| Sprite     | (invisible placeholder) |
| Tics       | -1                      |
| Next state | (this index)            |

#### Thing

| Field       | Value    |
|-------------|----------|
| Fast speed  | -1       |
| Melee range | 64.0     |

#### Sprite

No defaults.

#### Sound

| Field    | Value |
|----------|-------|
| Priority | 127   |
| Pitch    | -1    |
| Volume   | -1    |

### Defining Things / States / Sounds

You can define these as you normally do, with `Thing 1234`, `Frame 4444`, or `Sound 137` for instance. From the perspective of dehacked format, there is no change except conceptually "every index exists" for these entities. There is no corresponding `Sprite 111` block in dehacked, but sprites can be defined in the table discussed below.

### Defining Sound / Sprite Lumps

You can set the lump names for sprites and sounds in the table sections of the dehacked file. These sections normally replace lump names (OLD1 = NEW1). If the left value is a number, it is interpreted as the index where you want to store the given name. You _must_ set the lump names for any new sound or sprite indices that you reference elsewhere, otherwise the data won't be allocated and the associated sound / sprite won't actually exist. Here is an example:

```
[SPRITES]
1234 = NEW1
63 = WHOA

[SOUNDS]
100 = EXPLOD
3930 = TWISTR
```
