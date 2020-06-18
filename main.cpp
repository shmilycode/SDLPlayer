#include <SDL2/SDL.h>
#include "video/multicast_video_factory.h"
#include "player/sdl_player.h"
#include <unistd.h>
#include <termios.h>

using namespace VideoPlayer;
char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

int main(int argc, char** argv) {

  std::unique_ptr<MulticastVideoFactory> video_factory =
    std::make_unique<MulticastVideoFactory>();
  video_factory->OpenStream("239.0.0.105:6666");
  
  SDLConfig cfg{640, 480, SDL_PIXELFORMAT_IYUV};
  std::unique_ptr<SDLPlayer> sdl_player = 
    std::make_unique<SDLPlayer>(cfg, video_factory.get());
  sdl_player->Play();

  char input;
  while((input=getch()) == 'q')
    break;
  sdl_player->Stop();
  video_factory->CloseStream();
  return 0;
}
