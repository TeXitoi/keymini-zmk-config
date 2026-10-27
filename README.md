# ZMK config for keymini

ZMK firmware with ZMK Studio.

See the [keymini github](https://github.com/TeXitoi/keymini/) for more
information about keymini.

Erase ZMK Studio storage (if needed, at least the first time) with:

```
dfu-util -w -d 0483:df11 -a 0 -s 0x0801e800 -D keymini-erase-settings.bin
```

Flash ZMK with:

```
dfu-util -w -d 0483:df11 -a 0 -s 0x08000000:leave -D keymini-zmk.bin
```

After any modification, you can get the firmware corresponding to the
commit in the Action tab → select the commit → Artifacts
