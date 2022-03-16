//Multiplayer
#include <WS2tcpip.h>
#include <fstream>
#include <algorithm>

//Deklarowanie bibliotek
#include <Windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <mmsystem.h>
#include <cstdlib>
#include <time.h>
#include <string>

using namespace std;

//Multiplayer

WSAData wsadata;
SOCKET mySocket, clientSocket;
fstream file;
WIN32_FIND_DATA finder;
string line, listening, ip;
int port, playerNumber = 0;
sockaddr_in service, client;
char buf[30];
int sizeofclient = sizeof(client);


//Tworzenie zmiennych
SDL_Window *window;
SDL_Surface *screen;
SDL_Rect player[16];
SDL_Event event;
SDL_Surface * surface[10];
DWORD WINAPI PlayMusic(LPVOID) { while (true) { PlaySound("resource\\menu\\thememusic.wav", NULL, SND_SYNC); } }
int jump[2], score[2];
bool isjumping[2], onground[2], rendered, start, gamestarted, fullscreen;
SDL_Surface *text;