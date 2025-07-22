# wave-viewer

`wave-viewer` is a small terminal program that displays raw waveform data. It reads unsigned byte or 16‑bit samples from a file or standard input and provides simple interactive controls to inspect the data.

## Build

Use the bundled `Makefile`.

```sh
$ make
$ sudo make install  # optional
```

## Usage

```sh
$ wv filename
```

Passing `-` instead of a file name makes the program read from standard input. When fed with an endless stream the application will keep allocating memory for buffered data until it is terminated.

### Controls

* `h`, `l` – pan waveform left or right
* `i`, `o` – zoom in/out on the time axis
* `I`, `O` – zoom in/out on the amplitude axis
* `p` – pause rendering
* `q` – quit

## Third‑party code

The following source files were copied from other open‑source projects and remain under their respective MIT licenses:

* `argparse.hpp` &ndash; from [p-ranav/argparse](https://github.com/p-ranav/argparse).
* The `termbox/` directory &ndash; from [nsf/termbox](https://github.com/nsf/termbox).

Refer to the comments in each file for details.

## License

This project is distributed under the MIT License. See [LICENSE.md](LICENSE.md) for the full license text.
