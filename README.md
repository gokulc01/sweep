## Install
- Run `make build`, it generates executable sweep in working directory.
- Run `make install`, it generates executable sweep in `~/.local/bin/`, if `~/.local/bin/` is not in your PATH, copy the sweep executable to `/usr/bin/` or anywhere that is in PATH.


## Run
`sweep`
### Arguments
- `-(d|r|w|b)`:  modes
  - `-d (down)` : top to down sweep
  - `-r (right)` : left to right sweep
  - `-w (wiper)` : wiper sweep
  - `-b (blackhole)` : shrinking circle sweep
- `-t <num in ms>`: target animation time 

## Demo
### right
[![asciicast_right](https://asciinema.org/a/wB1QL0CdjdWl78N4VKEBpZa8d.svg)](https://asciinema.org/a/wB1QL0CdjdWl78N4VKEBpZa8d)

### down
[![asciicast_down](https://asciinema.org/a/EUblgEg5T0zBzn3k5ztQOUf63.svg)](https://asciinema.org/a/EUblgEg5T0zBzn3k5ztQOUf63)

### wiper
[![asciicast_wiper](https://asciinema.org/a/JsyNHB4xsp0oh5mOLMLJvPhAj.svg)](https://asciinema.org/a/JsyNHB4xsp0oh5mOLMLJvPhAj)

### blackhole
[![asciicast](https://asciinema.org/a/NAxAsgi57yZXBh6s8dLG14A8D.svg)](https://asciinema.org/a/NAxAsgi57yZXBh6s8dLG14A8D)
