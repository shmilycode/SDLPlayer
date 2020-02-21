##This is an example program for SDL2. 

All SDL function call has been packed in `SDLPlayer` class. What you need to do is inherit from class `VideoFactory`, implement its most important function `GetAvailableFrame`, which will be call in `SDLPlayer` to get new frame.

Only Support YUV420P pixel format data, and frame should be packed in `VideoFrame` structure to pass to `SDLPlayer` in `GetAvailableFrame`.

The main implementation of SDLPlayer is reference from ffplay in ffmpeg.

To initialize `SDLPlayer`, pass the windows configuration, and an instance of class who's parent class is `VideoFactory`. And just `Play` and `Stop` it.