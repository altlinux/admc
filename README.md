
# Building

See .gear/admc.spec for required packages.
- "BuildRequires:" - packages required for building the app.
- "Requires:" - packages required for running the app.

Once required packages are installed, run this from the admc folder:
```
$ mkdir build
$ cd build
$ cmake ..
$ make -j12
```

Note that currently ADMC is only guaranteed to be buildable on ALTLinux.

# Usage:

This app requires a working Active Directory domain and for the client machine to be connected and logged into the domain. You can find articles about these topics on [ALTLinux wiki](https://www.altlinux.org/%D0%94%D0%BE%D0%BC%D0%B5%D0%BD).

Launch admc from the build directory:
```
$ ./admc
```

# Testing

Tests also require a domain and a connection to the domain.

Launch tests from the build directory:
```
$ ./admc-test
```

# Screenshots

![image](https://i.imgur.com/GuRmwnq.png)
