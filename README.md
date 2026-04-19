# About

A very simple text-editor, inspired by the popular editors like **vim** and **nano**.

This editor uses vim like keybindings.

**NOTE!** Still in development.

## Requirements

This editor is built using the `ncurses` library and it is important to have it installed.

Most distros have this `ncurses` library pre-packaged by deafault.

### Debian/Ubuntu/Linux Mint

```console
sudo apt-get install libncurses5-dev libncursesw5-dev
```

### CentOS/RHEL/Fedora Linux

```console
sudo yum install ncurses-devel
```

### Arch

```console
sudo pacman -Syu ncurses
```

### Windows

For this only using `pdcurses` is a must. The installation process is kinda tedious. So shifting
to Linux will be a good option. Just run this program in a Linux like environment. WSL works.

I tried to fix this problem but it seems to annoy me alot.
That is why I shifted my development for this project in Linux.

## Build and Run

Either you run this simple one-liner command:

```console
gcc editor.c main.c -o kepler -lncurses -lm
```

Or build the project using `make`

```console
make -B
./kepler dummy.txt
...
```
