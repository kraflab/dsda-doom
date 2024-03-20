## Text Color Configuration

Text Color configurations are stored in the DSDATC lump. These defaults can be changed by replacing the lump.

### Specification

Each line of the DSDATC lump has a text component and a color index:

`text_component color_index`

Various elements of the extended hud, the automap, and the menus can be changed. The color indices correspond to entries in the [DSDACR lump](./color_range.md).

You can find the current default configuration [here](../prboom2/data/lumps/dsdatc.lmp).
