## Color Range Configuration

Color Range configurations are stored in the DSDACR lump. These defaults can be changed by replacing the lump. The ranges are used to translate font characters into different colors for the extended hud and menu text - manually created translation tables are no longer needed. Since the final colors are still constrained by what is available in the palette, custom color ranges may be preferable to the defaults (for instance, if a palette has no reds but has a rich range of pink).

### Specification

Each line of the DSDACR lump is plain text in the following format:

`color_range_index r1 g1 b1 r2 g2 b2 comment`

The `color_range_index` specifies which range will be replaced. The color range extends from (`r1`, `g1`, `b1`) to (`r2`, `g2`, `b2`) in the `rgb` color space. The `comment` field is optional and only exists for convenience (e.g., to label the colors). You can find the current default configuration [here](../prboom2/data/lumps/dsdacr.lmp).
