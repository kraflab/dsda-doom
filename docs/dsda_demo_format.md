## DSDA Demo Format

This port-specific demo format has some helpful extensions for casual play and some necessary extensions for advanced map formats. When using casual extensions, this format has no long-term _guarantee_ of cross-version compatibility and the resulting demos will not be accepted for upload to DSDA. The main purpose in this case is to assist in testing and FDAs.

### Header

The header follows a similar pattern to the old umapinfo header: the format is identified by version 255 and a unique signature, and the header is followed by an old format header. For example, if a demo is recorded in complevel 9, byte 16 will be 202 and will start boom's demo header. Byte 7 specifies the format version, and will be incremented when the demo format is updated (e.g., to add more data to the header).

| Byte(s) | Meaning                       |
| ------- | ----------------------------- |
| 0       | Version (0xff)                |
| 1-6     | Signature (0x1d D S D A 0xe6) |
| 7       | Format version                |
| 8-11    | Location of demo end marker   |
| 12-15   | Number of tics in demo        |
| 16+     | Complevel-specific header     |

### Tic

Tics are recorded with the complevel default data first (forwardmove, sidemove, etc). This data is followed by a byte indicating extra actions as flags. If the flag is associated with more data, it is appended to the tic. If there is no extra data, then there won't be any extra bytes in the tic. This means that tics don't have a constant length.

| Byte(s)   | Meaning                    |
| --------- | -------------------------- |
| 0-(x - 1) | Complevel / format default |
| x         | Action byte                |
| (x + 1)+  | Action data                |

| Action Bit | Meaning  |
| ---------- | -------- |
| 0          | Jump     |
| 1          | Save     |
| 2          | Load     |
| 3          | God Mode |
| 4          | No Clip  |
| 5+         | Reserved |

Save & Load are followed by 1 byte each that signifies the save slot used. Bits apply extra data in order and actions aren't mutually exclusive. This means that `0x06` signifies a save and a load on the same frame, and it would be followed by the save slot first and the load slot second.
