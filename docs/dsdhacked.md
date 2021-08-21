# DSDHacked

DSDA-Doom supports "unlimited" states, sprites, sounds, and things in dehacked (until you run out of memory / reach 2^32). This is a specification of the behaviour.

### Defaults

When you define a new index, the game allocates new entities up to that value. The data is filled with default values, listed here. Anything not specifically mentioned is set to zero / null.

#### State

| Field     | Value                   |
|-----------|-------------------------|
| sprite    | (invisible placeholder) |
| tics      | -1                      |
| nextstate | (this index)            |

#### Thing

No defaults.

#### Sprite

No defaults.

#### Sound

| Field    | Value |
|----------|-------|
| priority | 127   |
| pitch    | -1    |
| volume   | -1    |

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
