# uxnds

Quick and simple port of the [uxn virtual machine](https://wiki.xxiivv.com/site/uxn.html) to the
NDS and 3DS consoles.

You can download the latest version and source code [here](https://github.com/asiekierka/uxnds/releases).

## Usage

By default, uxnds will run /uxn/boot.rom or /uxn/launcher.rom. It also supports reading files from within /uxn.

On start, a keyboard is presented on the bottom screen, and the uxn display - on the top screen.
Use the L or R buttons to swap them - in this configuration, mouse input is approximated via 
touchscreen.

You can use the system button in the lower-left corner to reset the uxn virtual machine.

## Installation

Two ports are provided: the 3DS port (compatible with 3DS consoles) and the NDS port (compatible with DS, DSi and 3DS consoles).

### 3DS port

There is one binary provided: uxnds.3dsx.

To compile uxnds for 3DS, you may use:

* the latest devkitARM toolchain from the devkitPro organization to compile. After [installing](https://devkitpro.org/wiki/Getting_Started), simply run `make -f Makefile.3ds`.

### NDS port

When using a real DS, DSi or 3DS console, it is recommended to launch this program via
[nds-hb-menu](https://github.com/devkitPro/nds-hb-menu) - though, as it currently doesn't use argc/argv,
it doesn't really change much.

There are three binaries provided:

* uxnds.nds - faster, but best used only with known-good software,
* uxnds_debug.nds - slower, but provides debugging information, profiling information and performs CPU stack bounds checks.
* uxnds_profile.nds - almost as fast as uxnds.nds - with debugging/profiling information, no CPU stack bounds checks.

To compile uxnds for NDS, you may use:

* the [BlocksDS toolchain](https://github.com/blocksds/sdk) - run `make -f Makefile.blocksds`. This is the recommended path and is used by recent release builds;
* the latest devkitARM toolchain from the devkitPro organization to compile. After [installing](https://devkitpro.org/wiki/Getting_Started), run `make -f Makefile.nds`.
