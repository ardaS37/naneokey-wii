# Nane Okey Wii

Nane Okey Wii is an independent offline Nintendo Wii homebrew port of Nane Okey. It provides one local player and three bot opponents with a Wii Remote-driven interface.

## Features

- Offline single-console play: one human player and three bots.
- Wii Remote controls and a lightweight text/GUI interface.
- No LAN or online services.
- Built with C++ for Wii homebrew using devkitPro, libogc, and GRRLIB.

## Requirements

- devkitPro with `devkitPPC`
- `libogc`
- GRRLIB 4.6.1

## Build

```bash
cd wii
make
```

On Windows, `build-wii.cmd` is included as a convenience script. Its installation paths may need to be adjusted for your devkitPro setup.

## Controls

- **D-Pad** — move the cursor or change focus
- **A** — select or place a tile
- **B** — pick up the selected table tile
- **1** — create a new meld from selected hand tiles
- **+** — confirm the move
- **-** — draw a tile, or pass when the deck is empty
- **HOME** — start a new game

## Install on Wii

Run `package-homebrew.cmd` after building, then copy the resulting `apps/naneokey` folder to the root of an SD card for use with the Homebrew Channel.

## Notes

Build artifacts, packaged `.dol` files, object files, bundled GRRLIB sources, and downloaded archives are intentionally excluded from this source repository.
