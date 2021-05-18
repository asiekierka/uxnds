# uxnds

Quick and simple port of the [uxn virtual machine](https://wiki.xxiivv.com/site/uxn.html) to the
NDS console.

By default, uxnds will run /uxn/boot.rom. It also supports reading files from within /uxn.

On start, a keyboard is presented on the bottom screen, and the uxn display - on the top screen.
Use the L or R buttons to swap them - in this configuration, mouse input is approximated via 
touchscreen.

Use the latest devkitARM toolchain from the devkitPro organization to compile.
