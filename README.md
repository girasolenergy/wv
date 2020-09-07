# wv
wv is a wave viewing program that can view a single channel byte (unsigned char) file.

### Installation
```sh
$ make
$ sudo make install
```

### Usage
The usage is very straight forward. Use the file name as input to **`wv`**.
```sh
$ wv filename
```
**`filename`** can be a normal sized file, or a streaming file that never ends.
In the latter case, **`wv`** will show the data lively. Note that if you leave **`wv`** plotting stream data, it will continueously consume memory, until no memory is available and linux kernel will kill it.

**`wv`** uses interactive mode like **`vim`**. Here are some command you can use:
*  h, l, pan waveform left or right
*  i, o, zoom in or out in time
*  I, O, zoom in or out in y axis
*  q, quit
