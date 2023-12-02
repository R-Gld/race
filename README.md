# Race Game

This repository contains the source code for a simple game called "Race". The game is implemented in C language.

## Description

The game involves a player moving on a grid towards an objective point. The grid is represented as a 2D array, with each cell containing a value that can represent a bonus or a malus for the player. The player's movement is determined by a simple logic that moves the player towards the objective point.

## Files

The repository contains the following files:

- `main.c`: This is the main file that contains the game logic.
- `Makefile`: This file is used by Make to build the project.
- `.gitignore`: This file specifies intentionally untracked files that Git should ignore.

## Building

The project uses Make for building. To build the project, navigate to the project directory and run the following command:

```bash
make
```

This will generate an executable file named `main`.

## Running

To run the game, navigate to the project directory and run the following command:

```bash
make run
```

This command will execute the `race-server`, `race-mid`, `race-dumb`, and the `main` executable.

## Contributing

Contributions are welcome. Please feel free to fork the repository and submit pull requests.

## License

This project is licensed under the MIT License.