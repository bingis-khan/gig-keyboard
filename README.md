# My split keyboard

my cool split keyboard. contains OpenSCAD source files and the arduino project file.

![keyboard top](./keyboard-top.jpg)
![keyboard bottom](./keyboard-bot.jpg)

## Compiling

To generate the STL files, run `./print-script.sh`, which exports the STL part files.

For programming the board, use:

```
  arduino-cli compile -ub rp2040:rp2040:waveshare_rp2040_pizero -p /dev/ttyACM0
```


## Special mapping

- Soya button + hjkl - arrows
- Soya + top row - numbers
- Soya + 250mL + top row - Fn keys.
- 250mL (without Soya) - left alt
- juice key + -.'= keys - mouse movement.
- juice key + jk - scroll
- juice key + ,l/ - left/middle/right mouse button click


## Notes for V2

- use a pcb next time - wire crosstalk problems (actually, probably single copper strands touching each other)
- with psb, try adding how-swappable sockets.
- use linear switches
