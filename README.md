# zippy_dl
A simple program to download files from zippyshare.

# [How to use]
Please type `zippy_dl -h` to display help  
example: 

    zippy_dl http://www53.zippyshare.com/v/some/file.html http://www53.zippyshare.com/v/some_other/file.html

# [build] 

    git clone https://github.com/hare1039/zippy_dl.git

    cd zippy_dl && mkdir build && cd build

    cmake ..

And that's it!

You can find the binary inside build/zippy_dl, and you can move it anywhere you like!

I use phantomjs. Please make sure you could run `phantomjs` in shell.
