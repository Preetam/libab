# libab
[![Circle CI](https://circleci.com/gh/Preetam/libab.svg?style=svg&circle-token=2aa19d53d438447eae03021c0e99571e8ceb5207)](https://circleci.com/gh/Preetam/libab) [![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://github.com/Preetam/libab/blob/master/LICENSE)

The [Background](https://github.com/Preetam/libab/blob/master/background.md) has details about this projects.

## Dependencies and Building

You will need a compiler that supports C++14. Linux, OS X, and FreeBSD are supported at the moment.
Builds may succeed with other BSDs, but they have not been verified.

If you're using FreeBSD, you will need to install `libexecinfo`. You can do that using `pkg`:

```sh
# pkg install libexecinfo
```

This repository uses submodules. The following will fetch them if you haven't done so already:

```sh
$ git submodule update --init --recursive
```

You will also need CMake version 3.0 or higher.

### Building

```sh
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo make install # Optional
```

## License

BSD (see LICENSE)
