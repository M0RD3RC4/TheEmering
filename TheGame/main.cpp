#include "header.h"

int main(int argc, char *argv[]) {
	
	//Multiplayer
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		MessageBox(GetConsoleWindow(), (TEXT("Can't start the winsock, ERROR #") + to_string(WSAGetLastError())).c_str(), (TEXT("ERROR #") + to_string(WSAGetLastError())).c_str(), NULL);
		return -1;
	}

	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mySocket == INVALID_SOCKET) {
		MessageBox(GetConsoleWindow(), (TEXT("Can't create the socket, ERROR #") + to_string(WSAGetLastError())).c_str(), (TEXT("ERROR #") + to_string(WSAGetLastError())).c_str(), NULL);
		WSACleanup();
		return -1;
	}

	if (FindFirstFile("config.cfg", &finder) == INVALID_HANDLE_VALUE) {
		file.open("config.cfg", ios::out);
		file << "game-listening=true" << endl;
		file << "game-ip=127.0.0.1" << endl;
		file << "game-port=2137";
		file.close();
	}

	for (int i = 0; i <= 2; i++) {
		file.open("config.cfg", ios::in);
		switch (i) {
		case 0:
			while (getline(file, line)) {
				if (line.find("game-listening=") != string::npos) {
					line.erase(0, 15);
					transform(line.begin(), line.end(), line.begin(), ::tolower);
					listening = line;
				}
			}
			if (listening.empty()) {
				MessageBox(GetConsoleWindow(), "Can't find [game-listening]", "ERROR #" + GetLastError(), GetLastError());
				WSACleanup();
				return -1;
			}
			break;
		case 1:
			while (getline(file, line)) {
				if (line.find("game-ip=") != string::npos) {
					line.erase(0, 8);
					ip = line;
				}
			}
			if (ip.empty()) {
				MessageBox(GetConsoleWindow(), "Can't find [game-ip]", "ERROR #" + GetLastError(), GetLastError());
				WSACleanup();
				return -1;
			}
			break;
		case 2:
			while (getline(file, line)) {
				if (line.find("game-port=") != string::npos) {
					line.erase(0, 10);
					port = stoi(line);
				}
			}
			if (port == NULL) {
				MessageBox(GetConsoleWindow(), "Can't find [game-port]", "ERROR #" + GetLastError(), GetLastError());
				WSACleanup();
				return -1;
			}
			break;
		}
		file.close();
	}

	service.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &service.sin_addr);
	service.sin_port = htons(port);

	if (listening == "true") {
		if (bind(mySocket, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
			MessageBox(GetConsoleWindow(), (TEXT("Can't bind the socket, ERROR #") + to_string(GetLastError())).c_str(), (TEXT("ERROR #") + to_string(GetLastError())).c_str(), NULL);
			closesocket(mySocket);
			WSACleanup();
			return -1;
		}

		if (listen(mySocket, 1) == SOCKET_ERROR) {
			MessageBox(GetConsoleWindow(), (TEXT("Can't listen the socket, ERROR #") + to_string(GetLastError())).c_str(), (TEXT("ERROR #") + to_string(GetLastError())).c_str(), NULL);
			closesocket(mySocket);
			WSACleanup();
			return -1;
		}
	}
	else {
		if (connect(mySocket, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
			MessageBox(GetConsoleWindow(), (TEXT("Can't connect to the socket, ERROR #") + to_string(GetLastError())).c_str(), (TEXT("ERROR #") + to_string(GetLastError())).c_str(), NULL);
			closesocket(mySocket);
			WSACleanup();
			return -1;
		}
	}

	//Zainicjowanie SDL
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();

	//Tworzenie okna
	window = SDL_CreateWindow("The Emering", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, NULL);
	screen = SDL_GetWindowSurface(window);

	start = true;
	gamestarted = false;
	fullscreen = false;

game:
	//Pozycje graczy:

	//T³o
	player[0].x = 0;
	player[0].y = 0;
	player[0].w = 1280;
	player[0].h = 720;

	//Start
	player[1].x = 362;
	player[1].y = 400;
	player[1].w = 556;
	player[1].h = 107;

	//Quit
	player[2].x = 362;
	player[2].y = 500;
	player[2].w = 556;
	player[2].h = 107;

	//Liczkik I
	player[3].x = 0;
	player[3].y = 25;

	//Gracz 1
	player[4].x = 593;
	player[4].y = 549;

	//Gracz 2
	player[5].x = 593;
	player[5].y = 549;

	//Meta
	player[6].x = 0;
	player[6].y = 0;

	//Punkty pierwszego gracz
	player[7].y = 28;
	player[7].x = 15;

	//Punkty drugiego gracza
	player[8].y = 28;
	player[8].x = 1235;

	//Platformy
	for (int i = 9; i <= 13; i++) player[i].y = 549 + (9 - i) * 100;

	//Licznik II
	player[14].x = 1220;
	player[14].y = 25;

	//Wersja
	player[15].x = 750;
	player[15].y = 672;

	//Reszta kodu
	srand(time(NULL));
	for (int i = 0; i <= 1; i++) {
		jump[i] = 0;
		if (gamestarted == false) score[i] = 0;
		isjumping[i] = false;
		onground[i] = true;
	}
	rendered = false;
	if (gamestarted == true) goto world;

	//W³¹czanie muzyki
	CreateThread(NULL, NULL, PlayMusic, NULL, NULL, NULL);
	if (listening != "true") playerNumber = 1;

	while (true) {
		//Wychodznie z gry
		if (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) return 0;

		//Menu
		if (GetAsyncKeyState(VK_DOWN)) {
			if (start == true) {
				player[1].w = player[1].w - 10;
				player[1].h = player[1].h - 10;
				player[2].w = player[2].w + 10;
				player[2].h = player[2].h + 10;
				start = false;
				goto pressed;
			}
		}
		if (GetAsyncKeyState(VK_UP)) {
			if (start == false) {
				player[2].w = player[2].w - 10;
				player[2].h = player[2].h - 10;
				player[1].w = player[1].w + 10;
				player[1].h = player[1].h + 10;
				start = true;
				goto pressed;
			}
		}

		//Gra
		if (GetAsyncKeyState(VK_RETURN)) {
		world:
			while (start == true) {
				gamestarted = true;
				if (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) return 0; //Wychodznie z gry

				//Fullscreen
				if (GetAsyncKeyState(VK_F11)) {
					if (fullscreen == false) {
						SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
						screen = SDL_GetWindowSurface(window);
						fullscreen = true;
					}
					else {
						SDL_SetWindowFullscreen(window, NULL);
						screen = SDL_GetWindowSurface(window);
						fullscreen = false;
					}
				}

				//Wychodznie do menu
				if (GetAsyncKeyState(VK_ESCAPE)) {
					gamestarted = false;
					goto game;
				}

				//Sterowanie Graczami
				if (GetAsyncKeyState('W') || GetAsyncKeyState(VK_UP)) if (isjumping[playerNumber] == false) isjumping[playerNumber] = true;
				for(int i = 0; i <= 1; i++)
				if (onground[i] == true) {
					if (isjumping[i] == true) {
						if (jump[i] == 10) {
							jump[i] = 0;
							isjumping[i] = false;
							onground[i] = false;
						}
						else {
							jump[i]++;
							player[4 + i].y = player[4 + i].y - 15;
						}
					}
				}
				if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT)) player[4 + playerNumber].x = player[4 + playerNumber].x - 10;
				if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) player[4 + playerNumber].x = player[4 + playerNumber].x + 10;
				if (player[4 + playerNumber].x >= 1233) player[4 + playerNumber].x = player[4 + playerNumber].x - 10;

				//Fizyka
				player[4].y = player[4].y + 5;
				player[5].y = player[5].y + 5;

				if (player[4].y > 549) {
					player[4].y = player[4].y - 5;
					onground[0] = true;
				}
				if (player[5].y > 549) {
					player[5].y = player[5].y - 5;
					onground[1] = true;
				}

				for (int i = 9; i <= 13; i++) {
					if ((player[4].y + 100) == player[i].y) {
						if (player[4].x >= (player[i].x - 47)) {
							if (player[4].x <= (player[i].x + 178)) {
								player[4].y = player[4].y - 5;
								onground[0] = true;
							}
						}
					}
					if ((player[5].y + 100) == player[i].y) {
						if (player[5].x >= (player[i].x - 47))
							if (player[5].x <= (player[i].x + 178)) {
								player[5].y = player[5].y - 5;
								onground[1] = true;
							}
					}
				}

				//Renderowanie
				surface[0] = IMG_Load("resource\\game\\background.bmp");
				SDL_BlitScaled(surface[0], NULL, screen, &player[0]);
				surface[1] = IMG_Load("resource\\game\\grass.bmp");
				SDL_BlitScaled(surface[1], NULL, screen, &player[0]);
				surface[2] = IMG_Load("resource\\game\\platform.bmp");

				//Multiplayer
				if (listening == "true") {
					line = 'x' + to_string(player[4].x) + 'y' + to_string(player[4].y);

					for (int i = 9; i <= 13; i++) {
						if (rendered == false) {
							player[i].x = 640 - (rand() % 320 + 1);
							line += '|' + to_string(player[i].x);
						}
						SDL_BlitSurface(surface[2], NULL, screen, &player[i]);
					}
					if (rendered) line += 'r';
					else {
						line += '|';
						rendered = true;
					}

					send(clientSocket, line.c_str(), line.size() + 1, NULL);

					recv(clientSocket, buf, 30, NULL);
					line = (const char*)buf;

					player[5].x = stoi(line.substr(line.find('x') + 1, line.find('y') - 1));
					player[5].y = stoi(line.substr(line.find('y') + 1));
				}
				else {
					line = 'x' + to_string(player[5].x) + 'y' + to_string(player[5].y);
					send(mySocket, line.c_str(), line.size() + 1, NULL);

					recv(mySocket, buf, 30, NULL);
					line = (const char*)buf;

					if (line.find('r') != string::npos) {
						player[4].x = stoi(line.substr(line.find('x') + 1, line.find('y') - 1));
						player[4].y = stoi(line.substr(line.find('y') + 1, line.find('r') - 1));
					}
					else {
						player[4].x = stoi(line.substr(line.find('x') + 1, line.find('y') - 1));
						player[4].y = stoi(line.substr(line.find('y') + 1, line.find('|') - 1));
						line.erase(line.find('x'), line.find('|') + 1);
					}
					for (int i = 9; i <= 13; i++) {
						if (line.find('|') != string::npos) {
							player[i].x = stoi(line.substr(0, line.find('|')));
							line.erase(0, line.find('|') + 1);
						}
						SDL_BlitSurface(surface[2], NULL, screen, &player[i]);
					}
				}

				//Dalsze renderowanie
				surface[3] = IMG_Load("resource\\game\\character1.bmp");
				SDL_BlitSurface(surface[3], NULL, screen, &player[4]);
				surface[4] = IMG_Load("resource\\game\\character2.bmp");
				SDL_BlitSurface(surface[4], NULL, screen, &player[5]);
				surface[5] = IMG_Load("resource\\game\\finish.bmp");
				SDL_BlitSurface(surface[5], NULL, screen, &player[0]);
				surface[6] = IMG_Load("resource\\game\\score.bmp");
				SDL_BlitSurface(surface[6], NULL, screen, &player[3]);
				surface[7] = TTF_RenderText_Blended(TTF_OpenFont("resource\\font.ttf", 48), to_string(score[0]).c_str(), { 0, 0, 0 });
				SDL_BlitSurface(surface[7], NULL, screen, &player[7]);
				surface[8] = IMG_Load("resource\\game\\score.bmp");
				SDL_BlitSurface(surface[8], NULL, screen, &player[14]);
				surface[9] = TTF_RenderText_Blended(TTF_OpenFont("resource\\font.ttf", 48), to_string(score[1]).c_str(), { 0, 0, 0 });
				SDL_BlitSurface(surface[9], NULL, screen, &player[8]);

				//Odœwie¿anie ekranu
				SDL_UpdateWindowSurface(window);
				SDL_Delay(16.7);

				//Zwalnianie pamiêci RAM
				for (int i = 0; i <= 9; i++) {
					SDL_FreeSurface(surface[i]);
				}

				//Liczenie punktów
				if (player[4].y <= 10) {
					score[0]++;
					if (score[0] == 10) {
						gamestarted = false;
						goto game;
					}
				}

				if (player[5].y <= 10) {
					score[1]++;
					if (score[1] == 10) {
						gamestarted = false;
						goto game;
					}
					goto game;
				}
				if (player[4].y <= 10) goto game;
			}
			if (start == false) return 0;
		}
	pressed:
		//Renderowanie ekranu
		surface[0] = IMG_Load("resource\\menu\\background.bmp");
		SDL_BlitSurface(surface[0], NULL, screen, &player[0]);
		surface[1] = IMG_Load("resource\\menu\\text0.bmp");
		SDL_BlitSurface(surface[1], NULL, screen, &player[0]);
		surface[2] = IMG_Load("resource\\menu\\text1.bmp");
		SDL_BlitScaled(surface[2], NULL, screen, &player[1]);
		surface[3] = IMG_Load("resource\\menu\\text2.bmp");
		SDL_BlitScaled(surface[3], NULL, screen, &player[2]);
		surface[4] = TTF_RenderText_Blended(TTF_OpenFont("resource\\font.ttf", 48), "v0.3 - Multiplayer edition", { 255, 255, 255 });
		SDL_BlitSurface(surface[4], NULL, screen, &player[15]);

		//Fullscreen
		if (GetAsyncKeyState(VK_F11)) {
			if (fullscreen == false) {
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
				screen = SDL_GetWindowSurface(window);
				fullscreen = true;
			}
			else {
				SDL_SetWindowFullscreen(window, NULL);
				screen = SDL_GetWindowSurface(window);
				fullscreen = false;
			}
		}

		//Odœwie¿anie ekranu
		screen = SDL_GetWindowSurface(window);
		SDL_UpdateWindowSurface(window);
		SDL_Delay(16.7);
		
		if (!clientSocket) clientSocket = accept(mySocket, (sockaddr*)&client, &sizeofclient);

		//Zwalnianie pamiêci RAM
		for (int i = 0; i <= 4; i++) {
			SDL_FreeSurface(surface[i]);
		}
	}
}
