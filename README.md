## Install
- Run `make build`, it generates executable sweep in working directory
- Run `make install`, it generates executable sweep in `~/.local/bin/`


## Run
`sweep`
### Arguments
- `-(d|r|w|b)`:  modes
  - `-d (down)` : top to down sweep
  - `-r (right)` : left to right sweep
  - `-w (wiper)` : wiper sweep
  - `-b (blackhole)` : shrinking circle sweep
- `-t <num in ms>`: target animation time 
