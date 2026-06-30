# basejump [Under Repair]

`basejump` is designed as a command-line calculator and base converter written
in C. Its intended purpose is to accept numbers and arithmetic expressions in
bases 2 through 36, evaluate them, and display the result in one or more output
bases.

> **Current status:** This project is under repair and does not compile at
> present. Some conversion, evaluation, and terminal-handling functions depend
> on specific libraries that are no longer available in the current
> development environment. These dependencies are being replaced with local
> implementations.

## Intended capabilities

- Conversion between numeric bases from 2 to 36
- Evaluation of expressions using `+`, `-`, `*`, and `/`
- Interactive input with live base conversion
- Batch processing from an input file
- Configurable input and output bases
- Expression history during an interactive session

## Building

The project cannot currently be built successfully. Once the dependency
replacement work is complete, the intended build command will be:

```sh
make
```

The intended output is a `basejump` executable in the project directory. The
following maintenance targets are already defined in the `Makefile`:

```sh
make clean
```

Build with debugging symbols once compilation has been restored:

```sh
make debug
```

## Intended usage

```text
./basejump [--inbase 2..36] [--obases BASES] [--inputfile FILE]
```

Options:

- `--inbase`: sets the base used to interpret input values.
- `--obases`: sets a comma-separated list of output bases.
- `--inputfile`: reads expressions from a file instead of standard input.

Once the build is operational, running the program without options will use
base 10 for input and bases 2, 10, and 16 for output.

### Examples

Start an interactive session using the defaults:

```sh
./basejump
```

Read hexadecimal expressions and display results in binary and decimal:

```sh
./basejump --inbase 16 --obases 2,10
```

Process expressions from a file:

```sh
./basejump --inbase 8 --obases 2,10,16 --inputfile expressions.txt
```

## Intended interactive commands

Once interactive operation has been restored, commands will begin with `:`:

- `:i16` changes the input base to 16.
- `:o2,10,16` changes the output bases.
- An empty command (`:` followed by Enter) prints the session history.

The interface is designed to use `Ctrl-D` to exit, Backspace to edit the
current value, and Escape to clear the current expression.

## Project structure

- `main.c` — application entry point
- `program.c` — program flow and mode selection
- `parser.c` — command-line argument parsing and validation
- `interactive.c` — interactive input and session commands
- `filemode.c` — batch processing from files
- `outputs.c` — user-facing output formatting
- `constants.h` — shared limits, defaults, and status codes
