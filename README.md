Clip2QR
=======

Software tools to render a QR code are a dime a dozen.  Clip2QR distinguishing attributes are:

1) It acts as a clipboard viewer, rendering the QR code immediately following the user copying the text to the clipboard.
2) The Windows variant is written in classic WIN32 API, meaning the entire executable is under 40kBytes without using any additional libraries.

There is also an FLTK variant that can ostensibly be compiled for a variety of platforms, including Windows, Linux, and MacOS.

## Usage

Run the program the background, copy the desired text to the clipboard, and a QR code will appear in the Clip2QR window.

Re-size the Clip2QR window as desired.

To change the ECC level, right-click the Clip2QR window and make your selection.

## Building

Retrieve the C library:

```
git submodule update --init --recursive
```

and then go to the respective subdirectory ("windows" or "fltk") and do a make:

```
make
```

## Limitations

The source code requires a C99 compiler; however, this should be a non-issue for compilers released in the past two decades.

