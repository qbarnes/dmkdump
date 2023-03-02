# Building Dmkdump

There are three ways to build `dmkdump`:

   * [The usual way](#building-dmkdump-natively), which creates a native binary for the system you're running on,
   * [Cross-building](#cross-building-dmkdump), or
   * [Cross-building using OCI containers](#cross-building-dmkdump-using-oci-containers) (docker or podman).


## Building Dmkdump Natively

To build `dmkdump` for the system you're running on, you may need
to install additional packages before you build.

### Installing packages

First, you'll need to install packages for C development.

On Fedora and RHEL-like distros, run:

```
$ sudo dnf install -y @c-development
```

On Debian, Ubuntu, and related distros, run:

```
$ sudo apt-get update
$ sudo apt-get install -y build-essential
```

Then when cloning this repo, make sure to use the `--recursive` flag like so:

```
$ git clone --recursive git@github.com:qbarnes/dmkdump.git
```

### Building natively

To build, change directory into the cloned repo and run:
```
$ make
```

The binary will be under the `build` directory.


## Cross-Building Dmkdump

Cross-building creates `dmkdump` binaries for these six platforms all
at once:

   * Linux (x86_64, 32-bit and 64-bit ARM),
   * MS-DOS, and
   * 32-bit and 64-bit Microsoft Windows

Cross-building is only supported while running on x86_64 Linux.

To cross-build, you'll need to install some additional software first.

### Installing software

On Fedora and RHEL-like distros, run:

```
$ sudo dnf install -y @c-development mingw32-gcc mingw64-gcc
```

On Debian, Ubuntu, and related distros, run:

```
$ sudo apt-get update
$ sudo apt-get install -y build-essential mingw-w64 gcc-aarch64-linux-gnu gcc-arm-linux-gnueabi
```

For MS-DOS cross-compiling, the unpackaged DJGPP compiler is required.
To install, run:
```
$ wget https://github.com/andrewwutw/build-djgpp/releases/download/v3.1/djgpp-linux64-gcc1020.tar.bz2
$ sudo tar -xvC /usr/local -f djgpp-linux64-gcc1020.tar.bz2
```

Add the cross-compiler to your search path with:
```
$ export PATH=/usr/local/djgpp/bin:$PATH
```

Then when cloning this repo, make sure to use the `--recursive` flag like so:

```
$ git clone --recursive git@github.com:qbarnes/dmkdump.git
```

### Cross-building

To cross-build for all platforms, change directory into the cloned
repo and run:
```
$ make -f Makefile.cross
```

Binaries will be found under the `build.*` directories.

Unfortunately, Fedora and related distros don't fully support
cross-building for ARM.  Packages for ARM cross-compilers are
available, but without the necessary cross-environment.
To exclude the ARM platforms when cross-building, run:

```
$ make -f Makefile.cross BUILDS="LINUX_X86_64 MSDOS MSWIN32 MSWIN64"
```


## Cross-Building Dmkdump using OCI Containers

Cross-building `dmkdump` with containers builds binaries for all six
platforms like cross-building above, but does not require installing
any additional software packages on your system beyond `git`,
`make`, and OCI tools (`docker` or `podman`).  On Fedora, building
with containers also allows ARM cross-builds.

Cross-building with containers is only supported when running on
x86_64 Linux.

### Installing software

On Fedora and RHEL-like distros, run:

```
$ sudo dnf install -y git make podman
```

On Debian, Ubuntu, and related distros, run:

```
$ sudo apt-get update
$ sudo apt-get install -y git make docker
```

Then when cloning this repo, make sure to use the `--recursive` flag like so:

```
$ git clone --recursive git@github.com:qbarnes/dmkdump.git
```

### Using containers for building

```
$ make -f Makefile.oci
```
