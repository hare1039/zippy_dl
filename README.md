# zippy_dl
A simple program to download files from zippyshare.

[How to use]
example:
zippy_dl http://www53.zippyshare.com/v/some/file.html http://www53.zippyshare.com/v/some_other/file.html

[build]
I use curl, so link the libcurl when you compile.
And it need wget and a js interpreter too.

respberry pi3(raspbian) example:
g++ main.cpp -pthread -std=c++14 -L/usr/lib/arm-linux-gnueabihf/ -lcurl-nss -O2 -o zippy_dl

