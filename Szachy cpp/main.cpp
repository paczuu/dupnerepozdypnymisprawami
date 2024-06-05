#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

#define DLUGOSC 8
#define PIONEK_CZARNY 1
#define PIONEK_BIALY -1
#define WIEZA_CZARNA 2
#define WIEZA_BIALA -2
#define SKOCZEK_BIALY -3
#define SKOCZEK_CZARNY 3
#define GONIEC_CZARNY 4
#define GONIEC_BIALY -4
#define KROLOWA_BIALA -5
#define KROLOWA_CZARNA 5
#define KROL_CZARNY 6
#define KROL_BIALY -6

using namespace sf;

Font font;


struct poz {
    int x, y;
}staraPoz, krolBialy, krolCzarny, promocjaB, promocjaC, enPassantPoz;


int plansza[DLUGOSC][DLUGOSC] = { 
  2, 3, 4, 5, 6, 4, 3, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
 -1,-1,-1,-1,-1,-1,-1,-1,
 -2,-3,-4,-5,-6,-4,-3,-2, };

int rozmiar = 100, ruch = 0, x, y;

bool isPlayerVsPlayer = false; // okreœlanie trybu gry

int tura = 0; // 0- ruch bia³ego, 1- ruch czarnego

int wynik = 0; // 1- Wygrywaj¹ bia³e, 2- Wygrywaj¹ Czarne, 3- Remis

int szachBialy = 0, szachCzarny = 0; // 1- szach, 0- brak szachu

int roszadaBialaPrawo = 0, roszadaBialaLewo = 0, krolBialyRuch = 0;
int roszadaCzarnaPrawo = 0, roszadaCzarnaLewo = 0, krolCzarnyRuch = 0;

int promocjaBialy = 0, promocjaCzarny = 0; //1- mo¿liwa, 0- niemo¿liwa

int enPassant = 0; // 1- mo¿liwy, 0- niemo¿liwy


// Wyœwietlanie planszy
void wyswietlPlansze(RenderWindow& okno) {
    Color biale(235, 236, 208);
    Color czarne(119, 149, 86);

    for (int y = 0; y < DLUGOSC; ++y) {
        for (int x = 0; x < DLUGOSC; ++x) {
            RectangleShape pole(Vector2f(rozmiar, rozmiar));
            if ((x + y) % 2 == 0) {
                pole.setFillColor(biale);
            }
            else {
                pole.setFillColor(czarne);
            }
            pole.setPosition(x * rozmiar, y * rozmiar);
            okno.draw(pole);
        }
    }
}



// Manipulacja gr¹
void RestartGry() {
    std::fill(&plansza[0][0], &plansza[0][0] + sizeof(plansza) / sizeof(plansza[0][0]), 0);
    int initialSetup[8][8] = {
      2, 3, 4, 5, 6, 4, 3, 2,
      1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
     -1,-1,-1,-1,-1,-1,-1,-1,
     -2,-3,-4,-5,-6,-4,-3,-2, };
    std::copy(&initialSetup[0][0], &initialSetup[0][0] + 8 * 8, &plansza[0][0]);

    tura = 0;
    szachBialy = 0;
    szachCzarny = 0;
    promocjaBialy = 0;
    promocjaCzarny = 0;
    enPassant = 0;
    roszadaBialaPrawo = 0;
    roszadaBialaLewo = 0;
    krolBialyRuch = 0;
    roszadaCzarnaPrawo = 0;
    roszadaCzarnaLewo = 0;
    krolCzarnyRuch = 0;
    szachBialy = 0;
    szachCzarny = 0;
    wynik = 0;
}

void ZapiszGre() {
    std::ofstream plik;
    // Pobranie aktualnej daty i godziny
    time_t teraz = time(0);
    tm* ltm = localtime(&teraz);
    std::string nazwaPliku = std::to_string(1900 + ltm->tm_year) + "_" +
        std::to_string(1 + ltm->tm_mon) + "_" +
        std::to_string(ltm->tm_mday) + "_" +
        std::to_string(ltm->tm_hour) + "_" +
        std::to_string(ltm->tm_min) + "_" +
        std::to_string(ltm->tm_sec) + ".txt";
    plik.open("Zapisane gry/" + nazwaPliku);

    // Zapisanie planszy
    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            plik << plansza[i][j] << " ";
        }
        plik << "\n";
    }
    // Zapisanie kolejnoœci ruchu
    plik << tura << "\n";

    plik.close();
    std::cout << "Gra zapisana do pliku: " << nazwaPliku << std::endl;
}

void WczytajGre() {
    std::string sciezka;
    std::cout << "Podaj nazwe pliku: ";
    std::cin >> sciezka;

    // Dodanie rozszerzenia .txt jeœli nie zosta³o podane
    if (sciezka.size() < 4 || sciezka.substr(sciezka.size() - 4) != ".txt") {
        sciezka += ".txt";
    }

    // Dodanie œcie¿ki do katalogu
    sciezka = "Zapisane gry/" + sciezka;

    std::ifstream plik(sciezka);
    if (!plik) {
        std::cerr << "Nie mozna otworzyc pliku!" << std::endl;
        return;
    }

    for (int i = 0; i < DLUGOSC; ++i) {
        for (int j = 0; j < DLUGOSC; ++j) {
            plik >> plansza[i][j];
        }
    }
    plik >> tura;
    plik.close();
}



// Wszystkie ruchy figur
int PionBialy(int staryX, int staryY, int nowyX, int nowyY) {
    if (staryY == 6) {
        if ((nowyY == staryY - 1 && nowyX == staryX && plansza[staryY - 1][staryX] == 0) ||
            (nowyY == staryY - 2 && nowyX == staryX && plansza[staryY - 1][staryX] == 0 && plansza[staryY - 2][staryX] == 0)) {
            enPassant = 1;
            enPassantPoz = { staryX, staryY - 1 };
            return 1;
        }
    }
    else if (nowyY == staryY - 1 && nowyX == staryX && plansza[staryY - 1][staryX] == 0) {
        return 1;
    }
    if (plansza[staryY - 1][staryX - 1] > 0) {
        if (nowyY == staryY - 1 && nowyX == staryX - 1) {
            return 1;
        }
    }
    if (plansza[staryY - 1][staryX + 1] > 0) {
        if (nowyY == staryY - 1 && nowyX == staryX + 1) {
            return 1;
        }
    }
    // en passant
    if (enPassant && nowyY == enPassantPoz.y && nowyX == enPassantPoz.x && staryY == 3) {
        plansza[staryY][enPassantPoz.x] = 0;
        return 1;
    }
    return 0;
}

int PionCzarny(int staryX, int staryY, int nowyX, int nowyY) {
    if (staryY == 1) {
        if ((nowyY == staryY + 1 && nowyX == staryX && plansza[staryY + 1][staryX] == 0) ||
            (nowyY == staryY + 2 && nowyX == staryX && plansza[staryY + 1][staryX] == 0 && plansza[staryY + 2][staryX] == 0)) {
            enPassant = 1;
            enPassantPoz = { staryX, staryY + 1 };
            return 1;
        }
    }
    else if (nowyY == staryY + 1 && nowyX == staryX && plansza[staryY + 1][staryX] == 0) {
        return 1;
    }
    if (plansza[staryY + 1][staryX - 1] < 0) {
        if (nowyY == staryY + 1 && nowyX == staryX - 1) {
            return 1;
        }
    }
    if (plansza[staryY + 1][staryX + 1] < 0) {
        if (nowyY == staryY + 1 && nowyX == staryX + 1) {
            return 1;
        }
    }
    // en passant
    if (enPassant && nowyY == enPassantPoz.y && nowyX == enPassantPoz.x && staryY == 4) {
        plansza[staryY][enPassantPoz.x] = 0;
        return 1;
    }
    return 0;
}

int WiezaBiala(int staryX, int staryY, int nowyX, int nowyY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (plansza[staryY][i] >= 0 && (nowyX == i && nowyY == staryY)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (plansza[i][staryX] >= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i <= 7; i++) { // w prawo
        if (plansza[staryY][i] >= 0 && (nowyY == staryY && nowyX == i)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i <= 7; i++) { // w dó³
        if (plansza[i][staryX] >= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    return 0;
}

int WiezaCzarna(int staryX, int staryY, int nowyX, int nowyY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (plansza[staryY][i] <= 0 && (nowyX == i && nowyY == staryY)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (plansza[i][staryX] <= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i <= 7; i++) { // w prawo
        if (plansza[staryY][i] <= 0 && (nowyY == staryY && nowyX == i)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i <= 7; i++) { // w dó³
        if (plansza[i][staryX] <= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    return 0;
}

int GoniecBialy(int staryX, int staryY, int nowyX, int nowyY) {
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i <= 7; i++) { // w lewo w dó³
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i <= 7; i++) {  // w prawo w dó³
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int GoniecCzarny(int staryX, int staryY, int nowyX, int nowyY) {
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i <= 7; i++) { // w lewo w dó³
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i <= 7; i++) { // w prawo w dó³
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int KrolowaBiala(int staryX, int staryY, int nowyX, int nowyY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (plansza[staryY][i] >= 0 && (nowyX == i && nowyY == staryY)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (plansza[i][staryX] >= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i <= 7; i++) { // w prawo
        if (plansza[staryY][i] >= 0 && (nowyY == staryY && nowyX == i)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i <= 7; i++) { // w dó³
        if (plansza[i][staryX] >= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i <= 7; i++) { // w lewo w dó³
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i <= 7; i++) { // w prawo w dó³
        if (plansza[i][j] >= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int KrolowaCzarna(int staryX, int staryY, int nowyX, int nowyY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (plansza[staryY][i] <= 0 && (nowyX == i && nowyY == staryY)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (plansza[i][staryX] <= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i <= 7; i++) { // w prawo
        if (plansza[staryY][i] <= 0 && (nowyY == staryY && nowyX == i)) {
            return 1;
        }
        else if (plansza[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i <= 7; i++) { // w dó³
        if (plansza[i][staryX] <= 0 && (nowyY == i && nowyX == staryX)) {
            return 1;
        }
        else if (plansza[i][staryX] != 0) {
            break;
        }
    }
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i <= 7; i++) { // w lewo w dó³
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i <= 7; i++) { // w prawo w dó³
        if (plansza[i][j] <= 0 && (nowyY == i && nowyX == j)) {
            return 1;
        }
        else if (plansza[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int SkoczekBialy(int staryX, int staryY, int nowyX, int nowyY) {
    if (staryY - 2 >= 0 && staryX - 1 >= 0 && nowyY == staryY - 2 && nowyX == staryX - 1 && plansza[nowyY][nowyX] >= 0) {
        return 1; // w lewo w górê
    }
    if (staryY - 2 >= 0 && staryX + 1 < DLUGOSC && nowyY == staryY - 2 && nowyX == staryX + 1 && plansza[nowyY][nowyX] >= 0) {
        return 1; // w prawo w górê
    }
    if (staryY - 1 >= 0 && staryX + 2 < DLUGOSC && nowyY == staryY - 1 && nowyX == staryX + 2 && plansza[nowyY][nowyX] >= 0) {
        return 1; // w prawo 1
    }
    if (staryY + 1 >= 0 && staryX + 2 < DLUGOSC && nowyY == staryY + 1 && nowyX == staryX + 2 && plansza[nowyY][nowyX] >= 0) {
        return 1; // w prawo 2
    }
    if (staryY + 2 < DLUGOSC && staryX + 1 < DLUGOSC && nowyY == staryY + 2 && nowyX == staryX + 1 && plansza[nowyY][nowyX] >= 0) {
        return 1; // w dó³ 1
    }
    if (staryY + 2 < DLUGOSC && staryX - 1 >= 0 && nowyY == staryY + 2 && nowyX == staryX - 1 && plansza[nowyY][nowyX] >= 0) {
        return 1; //w dó³ 2
    }
    if (staryY + 1 < DLUGOSC && staryX - 2 >= 0 && nowyY == staryY + 1 && nowyX == staryX - 2 && plansza[nowyY][nowyX] >= 0) {
        return 1; // w lewo 1
    }
    if (staryY - 1 >= 0 && staryX - 2 >= 0 && nowyY == staryY - 1 && nowyX == staryX - 2 && plansza[nowyY][nowyX] >= 0) {
        return 1;
    }
    return 0;
}

int SkoczekCzarny(int staryX, int staryY, int nowyX, int nowyY) {
    if (staryY - 2 >= 0 && staryX - 1 >= 0 && nowyY == staryY - 2 && nowyX == staryX - 1 && plansza[nowyY][nowyX] <= 0) {
        return 1; // w lewo w górê
    }
    if (staryY - 2 >= 0 && staryX + 1 < DLUGOSC && nowyY == staryY - 2 && nowyX == staryX + 1 && plansza[nowyY][nowyX] <= 0) {
        return 1; // w prawo w górê
    }
    if (staryY - 1 >= 0 && staryX + 2 < DLUGOSC && nowyY == staryY - 1 && nowyX == staryX + 2 && plansza[nowyY][nowyX] <= 0) {
        return 1; // w prawo 1
    }
    if (staryY + 1 >= 0 && staryX + 2 < DLUGOSC && nowyY == staryY + 1 && nowyX == staryX + 2 && plansza[nowyY][nowyX] <= 0) {
        return 1; // w prawo 2
    }
    if (staryY + 2 < DLUGOSC && staryX + 1 < DLUGOSC && nowyY == staryY + 2 && nowyX == staryX + 1 && plansza[nowyY][nowyX] <= 0) {
        return 1; // w dó³ 1
    }
    if (staryY + 2 < DLUGOSC && staryX - 1 >= 0 && nowyY == staryY + 2 && nowyX == staryX - 1 && plansza[nowyY][nowyX] <= 0) {
        return 1; //w dó³ 2
    }
    if (staryY + 1 < DLUGOSC && staryX - 2 >= 0 && nowyY == staryY + 1 && nowyX == staryX - 2 && plansza[nowyY][nowyX] <= 0) {
        return 1; // w lewo 1
    }
    if (staryY - 1 >= 0 && staryX - 2 >= 0 && nowyY == staryY - 1 && nowyX == staryX - 2 && plansza[nowyY][nowyX] <= 0) {
        return 1;
    }
    return 0;
}



// Sprawdzanie ruchów w szachu
int PionBialySzach(int planszaArg[DLUGOSC][DLUGOSC], int posX, int posY, int krolX, int krolY) {
    if (planszaArg[posY - 1][posX - 1] >= 0) {
        if (posY - 1 == krolY && posX - 1 == krolX) {
            return 1;
        }
    }
    if (planszaArg[posY - 1][posX + 1] >= 0) {
        if (posY - 1 == krolY && posX + 1 == krolX) {
            return 1;
        }
    }
    return 0;
}

int WiezaBialaSzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (planszaArg[staryY][i] >= 0 && (krolX == i && krolY == staryY)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (planszaArg[i][staryX] >= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i < DLUGOSC; i++) { // w prawo
        if (planszaArg[staryY][i] >= 0 && (krolY == staryY && krolX == i)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w dó³
        if (planszaArg[i][staryX] >= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    return 0;
}

int GoniecBialySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w lewo w dó³
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) {  // w prawo w dó³
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int KrolowaBialaSzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (planszaArg[staryY][i] >= 0 && (krolX == i && krolY == staryY)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (planszaArg[i][staryX] >= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i < DLUGOSC; i++) { // w prawo
        if (planszaArg[staryY][i] >= 0 && (krolY == staryY && krolX == i)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w dó³
        if (planszaArg[i][staryX] >= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w lewo w dó³
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) {  // w prawo w dó³
        if (planszaArg[i][j] >= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int SkoczekBialySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    if (staryY - 2 >= 0 && staryX - 1 >= 0 && krolY == staryY - 2 && krolX == staryX - 1 && planszaArg[krolY][krolX] >= 0) {
        return 1; // w lewo w górê
    }
    if (staryY - 2 >= 0 && staryX + 1 < DLUGOSC && krolY == staryY - 2 && krolX == staryX + 1 && planszaArg[krolY][krolX] >= 0) {
        return 1; // w prawo w górê
    }
    if (staryY - 1 >= 0 && staryX + 2 < DLUGOSC && krolY == staryY - 1 && krolX == staryX + 2 && planszaArg[krolY][krolX] >= 0) {
        return 1; // w prawo 1
    }
    if (staryY + 1 >= 0 && staryX + 2 < DLUGOSC && krolY == staryY + 1 && krolX == staryX + 2 && planszaArg[krolY][krolX] >= 0) {
        return 1; // w prawo 2
    }
    if (staryY + 2 < DLUGOSC && staryX + 1 < DLUGOSC && krolY == staryY + 2 && krolX == staryX + 1 && planszaArg[krolY][krolX] >= 0) {
        return 1; // w dó³ 1
    }
    if (staryY + 2 < DLUGOSC && staryX - 1 >= 0 && krolY == staryY + 2 && krolX == staryX - 1 && planszaArg[krolY][krolX] >= 0) {
        return 1; //w dó³ 2
    }
    if (staryY + 1 < DLUGOSC && staryX - 2 >= 0 && krolY == staryY + 1 && krolX == staryX - 2 && planszaArg[krolY][krolX] >= 0) {
        return 1; // w lewo 1
    }
    if (staryY - 1 >= 0 && staryX - 2 >= 0 && krolY == staryY - 1 && krolX == staryX - 2 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    return 0;
}

int KrolBialySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    if (staryX - 1 >= 0 && staryY - 1 >= 0 && krolY == staryY - 1 && krolX == staryX - 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryY - 1 >= 0 && krolX == staryX && krolY == staryY - 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryY - 1 >= 0 && staryX + 1 < DLUGOSC && krolX == staryX + 1 && krolY == staryY - 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryX + 1 < DLUGOSC && krolY == staryY && krolX == staryX + 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryX + 1 < DLUGOSC && staryY + 1 < DLUGOSC && krolY == staryY + 1 && krolX == staryX + 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryY + 1 < DLUGOSC && krolY == staryY + 1 && krolX == staryX && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryX - 1 >= 0 && staryY + 1 < DLUGOSC && krolX == staryX - 1 && krolY == staryY + 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    if (staryX - 1 >= 0 && krolY == staryY && krolX == staryX - 1 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    return 0;
}

int PionCzarnySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    if (planszaArg[staryY + 1][staryX - 1] <= 0) {
        if (krolY == staryY + 1 && krolX == staryX - 1) {
            return 1;
        }
    }
    if (planszaArg[staryY + 1][staryX + 1] <= 0) {
        if (krolY == staryY + 1 && krolX == staryX + 1) {
            return 1;
        }
    }
    return 0;
}

int WiezaCzarnaSzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (planszaArg[staryY][i] <= 0 && (krolX == i && krolY == staryY)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (planszaArg[i][staryX] <= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i < DLUGOSC; i++) { // w prawo
        if (planszaArg[staryY][i] <= 0 && (krolY == staryY && krolX == i)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w dó³
        if (planszaArg[i][staryX] <= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    return 0;
}

int GoniecCzarnySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w lewo w dó³
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) {  // w prawo w dó³
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int KrolowaCzarnaSzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    for (int i = staryX - 1; i >= 0; i--) { // w lewo
        if (planszaArg[staryY][i] <= 0 && (krolX == i && krolY == staryY)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY - 1; i >= 0; i--) { // w górê
        if (planszaArg[i][staryX] <= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    for (int i = staryX + 1; i < DLUGOSC; i++) { // w prawo
        if (planszaArg[staryY][i] <= 0 && (krolY == staryY && krolX == i)) {
            return 1;
        }
        else if (planszaArg[staryY][i] != 0) {
            break;
        }
    }
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w dó³
        if (planszaArg[i][staryX] <= 0 && (krolY == i && krolX == staryX)) {
            return 1;
        }
        else if (planszaArg[i][staryX] != 0) {
            break;
        }
    }
    int j = staryX - 1;
    for (int i = staryY - 1; i >= 0; i--) { // w lewo w górê
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY - 1; i >= 0; i--) { // w prawo w górê
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    j = staryX - 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) { // w lewo w dó³
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j--;
    }
    j = staryX + 1;
    for (int i = staryY + 1; i < DLUGOSC; i++) {  // w prawo w dó³
        if (planszaArg[i][j] <= 0 && (krolY == i && krolX == j)) {
            return 1;
        }
        else if (planszaArg[i][j] != 0) {
            break;
        }
        j++;
    }
    return 0;
}

int SkoczekCzarnySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    if (staryY - 2 >= 0 && staryX - 1 >= 0 && krolY == staryY - 2 && krolX == staryX - 1 && planszaArg[krolY][krolX] <= 0) {
        return 1; // w lewo w górê
    }
    if (staryY - 2 >= 0 && staryX + 1 < DLUGOSC && krolY == staryY - 2 && krolX == staryX + 1 && planszaArg[krolY][krolX] <= 0) {
        return 1; // w prawo w górê
    }
    if (staryY - 1 >= 0 && staryX + 2 < DLUGOSC && krolY == staryY - 1 && krolX == staryX + 2 && planszaArg[krolY][krolX] <= 0) {
        return 1; // w prawo 1
    }
    if (staryY + 1 >= 0 && staryX + 2 < DLUGOSC && krolY == staryY + 1 && krolX == staryX + 2 && planszaArg[krolY][krolX] <= 0) {
        return 1; // w prawo 2
    }
    if (staryY + 2 < DLUGOSC && staryX + 1 < DLUGOSC && krolY == staryY + 2 && krolX == staryX + 1 && planszaArg[krolY][krolX] <= 0) {
        return 1; // w dó³ 1
    }
    if (staryY + 2 < DLUGOSC && staryX - 1 >= 0 && krolY == staryY + 2 && krolX == staryX - 1 && planszaArg[krolY][krolX] <= 0) {
        return 1; //w dó³ 2
    }
    if (staryY + 1 < DLUGOSC && staryX - 2 >= 0 && krolY == staryY + 1 && krolX == staryX - 2 && planszaArg[krolY][krolX] <= 0) {
        return 1; // w lewo 1
    }
    if (staryY - 1 >= 0 && staryX - 2 >= 0 && krolY == staryY - 1 && krolX == staryX - 2 && planszaArg[krolY][krolX] <= 0) {
        return 1;
    }
    return 0;
}

int KrolCzarnySzach(int planszaArg[DLUGOSC][DLUGOSC], int staryX, int staryY, int krolX, int krolY) {
    if (staryX - 1 >= 0 && staryY - 1 >= 0 && krolY == staryY - 1 && krolX == staryX - 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryY - 1 >= 0 && krolX == staryX && krolY == staryY - 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryY - 1 >= 0 && staryX + 1 < DLUGOSC && krolX == staryX + 1 && krolY == staryY - 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryX + 1 < DLUGOSC && krolY == staryY && krolX == staryX + 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryX + 1 < DLUGOSC && staryY + 1 < DLUGOSC && krolY == staryY + 1 && krolX == staryX + 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryY + 1 < DLUGOSC && krolY == staryY + 1 && krolX == staryX && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryX - 1 >= 0 && staryY + 1 < DLUGOSC && krolX == staryX - 1 && krolY == staryY + 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    if (staryX - 1 >= 0 && krolY == staryY && krolX == staryX - 1 && planszaArg[krolY][krolX] >= 0) {
        return 1;
    }
    return 0;
}


// Sprawdzanie czy wystêpuje szach
int KrolCzarnySzachCheck(int planszaArg[DLUGOSC][DLUGOSC], int krolX, int krolY) { // sprawdzanie czy wystepuje szach czarnego krola
    int ok = 0;
    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            if (planszaArg[i][j] < 0) {
                if (planszaArg[i][j] == PIONEK_BIALY) {
                    ok = PionBialySzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == WIEZA_BIALA) {
                    ok = WiezaBialaSzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == SKOCZEK_BIALY) {
                    ok = SkoczekBialySzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == GONIEC_BIALY) {
                    ok = GoniecBialySzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == KROLOWA_BIALA) {
                    ok = KrolowaBialaSzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == KROL_BIALY) {
                    ok = KrolBialySzach(planszaArg, j, i, krolX, krolY);
                }
                if (ok == 1) {
                    return 0; // jest szach
                }
            }
        }
    }
    return 1; //nie ma 
}

int KrolCzarny(int staryX, int staryY, int nowyX, int nowyY) {
    if (staryX - 1 >= 0 && staryY - 1 >= 0 && nowyY == staryY - 1 && nowyX == staryX - 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX - 1, staryY - 1);
        if (ok == 1) {
            return 1;  // w lewo w górê
        }
    }
    if (staryY - 1 >= 0 && nowyX == staryX && nowyY == staryY - 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX, staryY - 1);
        if (ok == 1) {
            return 1; // w górê
        }
    }
    if (staryY - 1 >= 0 && staryX + 1 < DLUGOSC && nowyX == staryX + 1 && nowyY == staryY - 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX + 1, staryY - 1);
        if (ok == 1) {
            return 1; // w prawo w górê
        }
    }
    if (staryX + 1 < DLUGOSC && nowyY == staryY && nowyX == staryX + 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX + 1, staryY);
        if (ok == 1) {
            return 1; // w prawo
        }
    }
    if (staryX + 1 < DLUGOSC && staryY + 1 < DLUGOSC && nowyY == staryY + 1 && nowyX == staryX + 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX + 1, staryY + 1);
        if (ok == 1) {
            return 1; // w prawo w dó³
        }
    }
    if (staryY + 1 < DLUGOSC && nowyY == staryY + 1 && nowyX == staryX && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX, staryY + 1);
        if (ok == 1) {
            return 1; // w dó³
        }
    }
    if (staryX - 1 >= 0 && staryY + 1 < DLUGOSC && nowyX == staryX - 1 && nowyY == staryY + 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX - 1, staryY + 1);
        if (ok == 1) {
            return 1; // w lewo w dó³
        }
    }
    if (staryX - 1 >= 0 && nowyY == staryY && nowyX == staryX - 1 && plansza[nowyY][nowyX] <= 0) {
        int ok = KrolCzarnySzachCheck(plansza, staryX - 1, staryY);
        if (ok == 1) {
            return 1; // w lewo
        }
    }
    // roszada w prawo
    if (roszadaCzarnaPrawo == 0 && krolCzarnyRuch == 0 && plansza[0][5] == 0 && plansza[0][6] == 0 && nowyY == 0 && nowyX == 6) {
        int ok = KrolCzarnySzachCheck(plansza, 4, 0);
        if (ok == 1) {
            ok = KrolCzarnySzachCheck(plansza, 5, 0);
            if (ok == 1) {
                ok = KrolCzarnySzachCheck(plansza, 6, 0);
                if (ok == 1) {
                    krolCzarnyRuch = 1;
                    roszadaCzarnaPrawo = 1;
                    plansza[0][7] = 0;
                    plansza[0][5] = WIEZA_CZARNA;
                    return 1;
                }
            }
        }
    }
    if (roszadaCzarnaLewo == 0 && krolCzarnyRuch == 0 && plansza[0][3] == 0 && plansza[0][2] == 0 && plansza[0][1] == 0 && nowyY == 0 && nowyX == 2) {
        int ok = KrolCzarnySzachCheck(plansza, 4, 0);
        if (ok == 1) {
            ok = KrolCzarnySzachCheck(plansza, 3, 0);
            if (ok == 1) {
                ok = KrolCzarnySzachCheck(plansza, 2, 0);
                if (ok == 1) {
                    ok = KrolCzarnySzachCheck(plansza, 1, 0);
                    if (ok == 1) {
                        krolCzarnyRuch = 1;
                        roszadaCzarnaLewo = 1;
                        plansza[0][0] = 0;
                        plansza[0][3] = WIEZA_CZARNA;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int KrolBialySzachCheck(int planszaArg[DLUGOSC][DLUGOSC], int krolX, int krolY) { // sprawdzanie czy wystepuje szach bialego krola
    int ok = 0;
    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            if (planszaArg[i][j] > 0) {
                if (planszaArg[i][j] == PIONEK_CZARNY) {
                    ok = PionCzarnySzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == WIEZA_CZARNA) {
                    ok = WiezaCzarnaSzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == SKOCZEK_CZARNY) {
                    ok = SkoczekCzarnySzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == GONIEC_CZARNY) {
                    ok = GoniecCzarnySzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == KROLOWA_CZARNA) {
                    ok = KrolowaCzarnaSzach(planszaArg, j, i, krolX, krolY);
                }
                if (planszaArg[i][j] == KROL_CZARNY) {
                    ok = KrolCzarnySzach(planszaArg, j, i, krolX, krolY);
                }
                if (ok == 1) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int KrolBialy(int staryX, int staryY, int nowyX, int nowyY) {
    if (staryX - 1 >= 0 && staryY - 1 >= 0 && nowyY == staryY - 1 && nowyX == staryX - 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX - 1, staryY - 1);
        if (ok == 1) {
            return 1;  // w lewo w górê
        }
    }
    if (staryY - 1 >= 0 && nowyX == staryX && nowyY == staryY - 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX, staryY - 1);
        if (ok == 1) {
            return 1; // w górê
        }
    }
    if (staryY - 1 >= 0 && staryX + 1 < DLUGOSC && nowyX == staryX + 1 && nowyY == staryY - 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX + 1, staryY - 1);
        if (ok == 1) {
            return 1; // w prawo w górê
        }
    }
    if (staryX + 1 < DLUGOSC && nowyY == staryY && nowyX == staryX + 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX + 1, staryY);
        if (ok == 1) {
            return 1; // w prawo
        }
    }
    if (staryX + 1 < DLUGOSC && staryY + 1 < DLUGOSC && nowyY == staryY + 1 && nowyX == staryX + 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX + 1, staryY + 1);
        if (ok == 1) {
            return 1; // w prawo w dó³
        }
    }
    if (staryY + 1 < DLUGOSC && nowyY == staryY + 1 && nowyX == staryX && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX, staryY + 1);
        if (ok == 1) {
            return 1; // w dó³
        }
    }
    if (staryX - 1 >= 0 && staryY + 1 < DLUGOSC && nowyX == staryX - 1 && nowyY == staryY + 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX - 1, staryY + 1);
        if (ok == 1) {
            return 1; // w lewo w dó³
        }
    }
    if (staryX - 1 >= 0 && nowyY == staryY && nowyX == staryX - 1 && plansza[nowyY][nowyX] >= 0) {
        int ok = KrolBialySzachCheck(plansza, staryX - 1, staryY);
        if (ok == 1) {
            return 1; // w lewo
        }
    }
    // roszada w prawo
    if (krolBialyRuch == 0 && roszadaBialaPrawo == 0 && plansza[7][5] == 0 && plansza[7][6] == 0 && nowyY == 7 && nowyX == 6) {
        int ok = 1;
        ok = KrolBialySzachCheck(plansza, 4, 7);
        if (ok == 1) {
            ok = KrolBialySzachCheck(plansza, 5, 7);
            if (ok == 1) {
                ok = KrolBialySzachCheck(plansza, 6, 7);
                if (ok == 1) {
                    plansza[7][7] = 0;
                    plansza[7][5] = WIEZA_BIALA;
                    krolBialyRuch = 1;
                    roszadaBialaPrawo = 1;
                    return 1;
                }
            }
        }
    }
    // roszada w lewo
    if (krolBialyRuch == 0 && roszadaBialaPrawo == 0 && plansza[7][3] == 0 && plansza[7][2] == 0 && plansza[7][1] == 0 && nowyY == 7 && nowyX == 2) {
        int ok = 1;
        ok = KrolBialySzachCheck(plansza, 4, 7);
        if (ok == 1) {
            ok = KrolBialySzachCheck(plansza, 3, 7);
            if (ok == 1) {
                ok = KrolBialySzachCheck(plansza, 2, 7);
                if (ok == 1) {
                    ok = KrolBialySzachCheck(plansza, 1, 7);
                    if (ok == 1) {
                        plansza[7][0] = 0;
                        plansza[7][3] = WIEZA_BIALA;
                        krolBialyRuch = 1;
                        roszadaBialaLewo = 1;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}


// Pobieranie aktualnej pozycji króla
poz PozycjaKrolBialy(int planszaArg[DLUGOSC][DLUGOSC]) {
    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            if (planszaArg[i][j] == KROL_BIALY) {
                return { j, i };
            }
        }
    }
}

poz PozycjaKrolCzarny(int planszaArg[DLUGOSC][DLUGOSC]) {
    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            if (planszaArg[i][j] == KROL_CZARNY) {
                return { j, i };
            }
        }
    }
}


// Obs³uga Matu i Patu
int CzarnyPatMat() {
    int kopiaPlansza[DLUGOSC][DLUGOSC];
    poz tempKrolCzarny;
    // Kopiowanie planszy
    std::copy(&plansza[0][0], &plansza[0][0] + DLUGOSC * DLUGOSC, &kopiaPlansza[0][0]);

    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            if (plansza[i][j] > 0) {
                for (int x = 0; x < DLUGOSC; x++) {
                    for (int y = 0; y < DLUGOSC; y++) {
                        int figura = plansza[i][j];
                        int ok = 0;

                        if (figura == PIONEK_CZARNY) {
                            ok = PionCzarny(j, i, x, y);
                        }
                        else if (figura == WIEZA_CZARNA) {
                            ok = WiezaCzarna(j, i, x, y);
                        }
                        else if (figura == SKOCZEK_CZARNY) {
                            ok = SkoczekCzarny(j, i, x, y);
                        }
                        else if (figura == GONIEC_CZARNY) {
                            ok = GoniecCzarny(j, i, x, y);
                        }
                        else if (figura == KROLOWA_CZARNA) {
                            ok = KrolowaCzarna(j, i, x, y);
                        }
                        else if (figura == KROL_CZARNY) {
                            ok = KrolCzarny(j, i, x, y);
                        }

                        if (ok == 1) {
                            // Wykonaj symulowany ruch na kopii planszy
                            int poprzedniaFigura = kopiaPlansza[y][x];
                            kopiaPlansza[y][x] = figura;
                            kopiaPlansza[i][j] = 0;

                            tempKrolCzarny = PozycjaKrolCzarny(kopiaPlansza);
                            int szach = KrolCzarnySzachCheck(kopiaPlansza, tempKrolCzarny.x, tempKrolCzarny.y);
                            if (szach == 1) {
                                return 0;
                            }

                            // Cofnij symulowany ruch
                            kopiaPlansza[i][j] = figura;
                            kopiaPlansza[y][x] = poprzedniaFigura;
                        }
                    }
                }
            }
        }
    }
    tura = 2;
    return 1; // je¿eli s¹ tylko ruchy powodujace szach lub nie ma ruchów legalnych
}

int BialyPatMat() {
    int kopiaPlansza[DLUGOSC][DLUGOSC];
    poz tempKrolBialy;
    // Kopiowanie planszy
    std::copy(&plansza[0][0], &plansza[0][0] + DLUGOSC * DLUGOSC, &kopiaPlansza[0][0]);

    for (int i = 0; i < DLUGOSC; i++) {
        for (int j = 0; j < DLUGOSC; j++) {
            if (plansza[i][j] < 0) {
                for (int x = 0; x < DLUGOSC; x++) {
                    for (int y = 0; y < DLUGOSC; y++) {
                        int figura = plansza[i][j];
                        int ok = 0;

                        if (figura == PIONEK_BIALY) {
                            ok = PionBialy(j, i, x, y);
                        }
                        else if (figura == WIEZA_BIALA) {
                            ok = WiezaBiala(j, i, x, y);
                        }
                        else if (figura == SKOCZEK_BIALY) {
                            ok = SkoczekBialy(j, i, x, y);
                        }
                        else if (figura == GONIEC_BIALY) {
                            ok = GoniecBialy(j, i, x, y);
                        }
                        else if (figura == KROLOWA_BIALA) {
                            ok = KrolowaBiala(j, i, x, y);
                        }
                        else if (figura == KROL_BIALY) {
                            ok = KrolBialy(j, i, x, y);
                        }

                        if (ok == 1) {
                            // Wykonaj symulowany ruch na kopii planszy
                            int poprzedniaFigura = kopiaPlansza[y][x];
                            kopiaPlansza[y][x] = figura;
                            kopiaPlansza[i][j] = 0;

                            tempKrolBialy = PozycjaKrolBialy(kopiaPlansza);
                            int szach = KrolBialySzachCheck(kopiaPlansza, tempKrolBialy.x, tempKrolBialy.y);
                            if (szach == 1) {
                                return 0;
                            }

                            // Cofnij symulowany ruch
                            kopiaPlansza[i][j] = figura;
                            kopiaPlansza[y][x] = poprzedniaFigura;
                        }
                    }
                }
            }
        }
    }
    tura = 2;
    return 2; // Wygrywaj¹ czarne
}


// Implementacja "AI"
struct Move {
    int startX, startY;
    int endX, endY;
};

int evaluateBoard(int plansza[DLUGOSC][DLUGOSC]) {
    int value = 0;
    for (int y = 0; y < DLUGOSC; ++y) {
        for (int x = 0; x < DLUGOSC; ++x) {
            value += plansza[y][x];
        }
    }
    return value;
}

int generateLegalMoves(int plansza[DLUGOSC][DLUGOSC], Move* moves) {
    int moveCount = 0;
    int tempPlansza[DLUGOSC][DLUGOSC];

    for (int y = 0; y < DLUGOSC; ++y) {
        for (int x = 0; x < DLUGOSC; ++x) {
            if (plansza[y][x] > 0) { // Tylko czarne figury
                for (int ny = 0; ny < DLUGOSC; ++ny) {
                    for (int nx = 0; nx < DLUGOSC; ++nx) {
                        bool ok = false;
                        if (plansza[y][x] == PIONEK_CZARNY) ok = PionCzarny(x, y, nx, ny);
                        else if (plansza[y][x] == WIEZA_CZARNA) ok = WiezaCzarna(x, y, nx, ny);
                        else if (plansza[y][x] == SKOCZEK_CZARNY) ok = SkoczekCzarny(x, y, nx, ny);
                        else if (plansza[y][x] == GONIEC_CZARNY) ok = GoniecCzarny(x, y, nx, ny);
                        else if (plansza[y][x] == KROLOWA_CZARNA) ok = KrolowaCzarna(x, y, nx, ny);
                        else if (plansza[y][x] == KROL_CZARNY) ok = KrolCzarny(x, y, nx, ny);

                        if (ok) {
                            // Skopiuj planszê do tymczasowej planszy
                            std::copy(&plansza[0][0], &plansza[0][0] + DLUGOSC * DLUGOSC, &tempPlansza[0][0]);

                            // Wykonaj tymczasowy ruch
                            tempPlansza[ny][nx] = tempPlansza[y][x];
                            tempPlansza[y][x] = 0;

                            // SprawdŸ, czy ruch nie powoduje szachu czarnego króla
                            poz krolCzarny = PozycjaKrolCzarny(tempPlansza);
                            bool szach = KrolCzarnySzachCheck(tempPlansza, krolCzarny.x, krolCzarny.y);

                            // Jeœli ruch nie powoduje szachu, dodaj go do listy ruchów
                            if (szach == 1) {
                                moves[moveCount++] = { x, y, nx, ny };
                            }
                        }
                    }
                }
            }
        }
    }
    return moveCount;
}

int minimax(int plansza[DLUGOSC][DLUGOSC], int depth, bool isMaximizing) {
    if (depth == 0) {
        return evaluateBoard(plansza);
    }

    Move moves[100];
    int moveCount = generateLegalMoves(plansza, moves);
    int bestValue = isMaximizing ? -9999 : 9999;

    for (int i = 0; i < moveCount; ++i) {
        Move move = moves[i];
        int backupPiece = plansza[move.endY][move.endX];
        plansza[move.endY][move.endX] = plansza[move.startY][move.startX];
        plansza[move.startY][move.startX] = 0;

        int value = minimax(plansza, depth - 1, !isMaximizing);

        plansza[move.startY][move.startX] = plansza[move.endY][move.endX];
        plansza[move.endY][move.endX] = backupPiece;

        if (isMaximizing) {
            bestValue = std::max(bestValue, value);
        }
        else {
            bestValue = std::min(bestValue, value);
        }
    }

    return bestValue;
}

Move getBestMove(int plansza[DLUGOSC][DLUGOSC], int depth) {
    Move moves[100];
    int moveCount = generateLegalMoves(plansza, moves);
    Move bestMove = moves[0];
    int bestValue = -9999;

    for (int i = 0; i < moveCount; ++i) {
        Move move = moves[i];
        int backupPiece = plansza[move.endY][move.endX];
        plansza[move.endY][move.endX] = plansza[move.startY][move.startX];
        plansza[move.startY][move.startX] = 0;

        int value = minimax(plansza, depth - 1, true);

        plansza[move.startY][move.startX] = plansza[move.endY][move.endX];
        plansza[move.endY][move.endX] = backupPiece;

        if (value > bestValue) {
            bestValue = value;
            bestMove = move;
        }
    }

    return bestMove;
}

void makeAiMove(int plansza[DLUGOSC][DLUGOSC]) {
    Move move = getBestMove(plansza, 2); // G³êbokoœæ przeszukiwania 2
    plansza[move.endY][move.endX] = plansza[move.startY][move.startX];
    plansza[move.startY][move.startX] = 0;
}



int main() {
    RenderWindow okno(VideoMode(800, 830), "Szachy", Style::Titlebar | Style::Close);; // 800x800 plansza + 30px na przyciski
    Texture grafika1, grafika2, grafika3, grafika4, grafika6, grafika5, grafika7, 
            grafika8, grafika10, grafika9, grafika11, grafika12, grafika14, grafika13;

    grafika1.loadFromFile("images/black-pawn.png");
    grafika2.loadFromFile("images/white-pawn.png");
    grafika3.loadFromFile("images/black-rook.png");
    grafika4.loadFromFile("images/white-rook.png");
    grafika5.loadFromFile("images/black-nightrd.png");
    grafika6.loadFromFile("images/white-nightrd.png");
    grafika7.loadFromFile("images/black-bishop.png");
    grafika8.loadFromFile("images/white-bishop.png");
    grafika9.loadFromFile("images/black-queen.png");
    grafika10.loadFromFile("images/white-queen.png");
    grafika11.loadFromFile("images/black-king.png");
    grafika12.loadFromFile("images/white-king.png");
    grafika13.loadFromFile("images/promocja_czarne.png");
    grafika14.loadFromFile("images/promocja_biale.png");
    
    Sprite PionC(grafika1);
    Sprite PionB(grafika2);
    Sprite WiezaC(grafika3);
    Sprite WiezaB(grafika4);
    Sprite SkoczekB(grafika6);
    Sprite SkoczekC(grafika5);
    Sprite GoniecC(grafika7);
    Sprite GoniecB(grafika8);
    Sprite KrolowaB(grafika10);
    Sprite KrolowaC(grafika9);
    Sprite KrolC(grafika11);
    Sprite KrolB(grafika12);
    Sprite Ruch;
    Sprite PromocjaBIALY(grafika14);
    Sprite PromocjaCZARNY(grafika13);

    font.loadFromFile("fonts/Cour.ttf");

    // Dodanie przycisków pod plansza
    RectangleShape zapiszPrzycisk(Vector2f(250, 30));
    zapiszPrzycisk.setFillColor(Color(0xDB, 0xB4, 0x5C));
    zapiszPrzycisk.setPosition(0, 800);

    RectangleShape poddajPrzycisk(Vector2f(300, 30));
    poddajPrzycisk.setFillColor(Color(0x9C, 0x8C, 0x68));
    poddajPrzycisk.setPosition(250, 800);

    RectangleShape wczytajPrzycisk(Vector2f(250, 30));
    wczytajPrzycisk.setFillColor(Color(0xDB, 0xB4, 0x5C));
    wczytajPrzycisk.setPosition(550, 800);

    RectangleShape zagrajPonowniePrzycisk(Vector2f(250, 30));
    zagrajPonowniePrzycisk.setFillColor(Color(0x6C, 0xA6, 0xCD));
    zagrajPonowniePrzycisk.setPosition(275, 500);


    Text zapiszTekst("Zapisz gre", font, 20);
    zapiszTekst.setFillColor(Color::Black);
    zapiszTekst.setPosition(60, 800);

    Text poddajTekst("Poddaj sie", font, 20);
    poddajTekst.setFillColor(Color::Black);
    poddajTekst.setPosition(335, 800);

    Text wczytajTekst("Wczytaj gre", font, 20);
    wczytajTekst.setFillColor(Color::Black);
    wczytajTekst.setPosition(610, 800);

    Text zagrajPonownieTekst("Zagraj Ponownie", font, 20);
    zagrajPonownieTekst.setFillColor(Color::Black);
    zagrajPonownieTekst.setPosition(310, 500);

    Text wynikTekst("", font, 50);
    wynikTekst.setFillColor(Color::Red);
    wynikTekst.setStyle(Text::Bold);
    
    FloatRect textRect = wynikTekst.getLocalBounds();
    wynikTekst.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    wynikTekst.setPosition(170, 400);

    float dx = 0, dy = 0;
    int wybranaFigura = 0;

    // Menu g³ówne
    while (okno.isOpen()) {
        Vector2i pos = Mouse::getPosition(okno);
        x = pos.x / rozmiar;
        y = pos.y / rozmiar;
        Event e;

        while (okno.pollEvent(e)) {
            if (e.type == Event::Closed) {
                okno.close();
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                okno.close();
            }
        }
        okno.clear();

        Text text("Wybierz tryb gry:\n1. Gracz vs Gracz\n2. Gracz vs Komputer", font, 24);
        text.setFillColor(Color::White);
        text.setPosition(200, 200);
        okno.draw(text);
        okno.display();

        if (Keyboard::isKeyPressed(Keyboard::Num1)) {
            isPlayerVsPlayer = true;
            break;
        }
        if (Keyboard::isKeyPressed(Keyboard::Num2)) {
            isPlayerVsPlayer = false;
            break;
        }
    }

    // G³ówna pêtla gry
    while (okno.isOpen()) {
        Vector2i pos = Mouse::getPosition(okno);
        x = pos.x / rozmiar;
        y = pos.y / rozmiar;
        Event e;

        while (okno.pollEvent(e)) {
            if (e.type == Event::Closed) {
                okno.close();
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                okno.close();
            }
            if (e.type == Event::MouseButtonPressed) {
                if (e.key.code == Mouse::Left) {
                    if (promocjaBialy == 1) { // wybór figury po promocji
                        if (pos.y >= promocjaB.y * rozmiar && pos.y <= (promocjaB.y + 1) * rozmiar && pos.x >= promocjaB.x * rozmiar && pos.x <= (promocjaB.x + 1) * rozmiar) {
                            int xx = pos.x % 100, yy = pos.y % 100; // numer pola
                            if (xx < 50 && yy < 50 && xx > 0 && yy > 0) {
                                plansza[promocjaB.y][promocjaB.x] = WIEZA_BIALA;
                                promocjaBialy = 0;
                            }
                            if (xx > 50 && xx < 100 && yy < 50 && yy > 0) {
                                plansza[promocjaB.y][promocjaB.x] = KROLOWA_BIALA;
                                promocjaBialy = 0;
                            }
                            if (xx > 50 && xx < 100 && yy > 50 && yy < 100) {
                                plansza[promocjaB.y][promocjaB.x] = SKOCZEK_BIALY;
                                promocjaBialy = 0;
                            }
                            if (xx < 50 && xx > 0 && yy > 50 && y < 100) {
                                plansza[promocjaB.y][promocjaB.x] = GONIEC_BIALY;
                                promocjaBialy = 0;
                            }
                            if (promocjaBialy == 0) {
                                krolCzarny = PozycjaKrolCzarny(plansza);
                                int h = KrolCzarnySzachCheck(plansza, krolCzarny.x, krolCzarny.y);
                                if (h == 0) {
                                    szachCzarny = 1;
                                }
                            }
                        }
                    }
                    if (promocjaCzarny == 1) { // wybór figury po promocji
                        if (pos.y >= promocjaC.y * rozmiar && pos.y <= (promocjaC.y + 1) * rozmiar && pos.x >= promocjaC.x * rozmiar && pos.x <= (promocjaC.x + 1) * rozmiar) {
                            int xx = pos.x % 100, yy = pos.y % 100; // numer pola
                            if (xx < 50 && yy < 50 && xx > 0 && yy > 0) {
                                plansza[promocjaC.y][promocjaC.x] = WIEZA_CZARNA;
                                promocjaCzarny = 0;
                            }
                            if (xx > 50 && xx < 100 && yy < 50 && yy > 0) {
                                plansza[promocjaC.y][promocjaC.x] = KROLOWA_CZARNA;
                                promocjaCzarny = 0;
                            }
                            if (xx > 50 && xx < 100 && yy > 50 && yy < 100) {
                                plansza[promocjaC.y][promocjaC.x] = SKOCZEK_CZARNY;
                                promocjaCzarny = 0;
                            }
                            if (xx < 50 && xx > 0 && yy > 50 && y < 100) {
                                plansza[promocjaC.y][promocjaC.x] = GONIEC_CZARNY;
                                promocjaCzarny = 0;
                            }
                            if (promocjaCzarny == 0) {
                                krolBialy = PozycjaKrolBialy(plansza);
                                int h = KrolBialySzachCheck(plansza, krolBialy.x, krolBialy.y);
                                if (h == 0) {
                                    szachBialy = 1;
                                }
                            }
                        }
                    }
                    if (plansza[y][x] != 0) { // wybór figury do ruchu
                        dx = pos.x - x * 100;
                        dy = pos.y - y * 100;
                        if (plansza[y][x] == PIONEK_CZARNY && tura == 1) {
                            wybranaFigura = PIONEK_CZARNY;
                            Ruch = PionC;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == PIONEK_BIALY && tura == 0) {
                            wybranaFigura = PIONEK_BIALY;
                            Ruch = PionB;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == WIEZA_CZARNA && tura == 1) {
                            wybranaFigura = WIEZA_CZARNA;
                            Ruch = WiezaC;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == WIEZA_BIALA && tura == 0) {
                            wybranaFigura = WIEZA_BIALA;
                            Ruch = WiezaB;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == SKOCZEK_BIALY && tura == 0) {
                            wybranaFigura = SKOCZEK_BIALY;
                            Ruch = SkoczekB;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == SKOCZEK_CZARNY && tura == 1) {
                            wybranaFigura = SKOCZEK_CZARNY;
                            Ruch = SkoczekC;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == GONIEC_CZARNY && tura == 1) {
                            wybranaFigura = GONIEC_CZARNY;
                            Ruch = GoniecC;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == GONIEC_BIALY && tura == 0) {
                            wybranaFigura = GONIEC_BIALY;
                            Ruch = GoniecB;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == KROLOWA_BIALA && tura == 0) {
                            wybranaFigura = KROLOWA_BIALA;
                            Ruch = KrolowaB;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == KROLOWA_CZARNA && tura == 1) {
                            wybranaFigura = KROLOWA_CZARNA;
                            Ruch = KrolowaC;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == KROL_CZARNY && tura == 1) {
                            wybranaFigura = KROL_CZARNY;
                            Ruch = KrolC;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == KROL_BIALY && tura == 0) {
                            wybranaFigura = KROL_BIALY;
                            Ruch = KrolB;
                            plansza[y][x] = 0;
                        }
                        if (plansza[y][x] == 0) {
                            ruch = 1;
                            staraPoz.x = x;
                            staraPoz.y = y;
                        }
                    }
                    else if (pos.y > 800 && 0 == tura == 1) {
                        if (pos.x < 250) {
                            ZapiszGre();
                        }
                        else if (pos.x < 550) {
                            std::cout << "Przeciwnik wygral!" << std::endl;
                            okno.close();
                        }
                        else if (pos.x < 800) {
                            WczytajGre();
                        }
                    }
                }
            }
            if (e.type == Event::MouseButtonReleased) {
                if (e.key.code == Mouse::Left) {
                    int ok = 2;
                    if (wybranaFigura == PIONEK_BIALY && ruch == 1) {
                        ok = PionBialy(staraPoz.x, staraPoz.y, x, y);
                        if (ok == 1 && enPassant && enPassantPoz.y == y && enPassantPoz.x == x) {
                            enPassant = 0;
                        }
                    }
                    if (wybranaFigura == PIONEK_CZARNY && ruch == 1) {
                        ok = PionCzarny(staraPoz.x, staraPoz.y, x, y);
                        if (ok == 1 && enPassant && enPassantPoz.y == y && enPassantPoz.x == x) {
                            enPassant = 0;
                        }
                    }
                    if (wybranaFigura == WIEZA_BIALA && ruch == 1) {
                        ok = WiezaBiala(staraPoz.x, staraPoz.y, x, y);
                        if (ok == 1 && roszadaBialaLewo == 0 && staraPoz.y == 7 && staraPoz.x == 0) {
                            roszadaBialaLewo = 1;
                        }
                        if (ok == 1 && roszadaBialaPrawo == 0 && staraPoz.y == 7 && staraPoz.x == 7) {
                            roszadaBialaPrawo = 1;
                        }
                    }
                    if (wybranaFigura == WIEZA_CZARNA && ruch == 1) {
                        ok = WiezaCzarna(staraPoz.x, staraPoz.y, x, y);
                        if (ok == 1 && roszadaCzarnaPrawo == 0 && staraPoz.y == 0 && staraPoz.x == 7) {
                            roszadaCzarnaPrawo = 1;
                        }
                        if (ok == 1 && roszadaCzarnaLewo == 0 && staraPoz.y == 0 && staraPoz.x == 0) {
                            roszadaCzarnaLewo = 1;
                        }
                    }
                    if (wybranaFigura == GONIEC_BIALY && ruch == 1) {
                        ok = GoniecBialy(staraPoz.x, staraPoz.y, x, y);
                    }
                    if (wybranaFigura == GONIEC_CZARNY && ruch == 1) {
                        ok = GoniecCzarny(staraPoz.x, staraPoz.y, x, y);
                    }
                    if (wybranaFigura == KROLOWA_BIALA && ruch == 1) {
                        ok = KrolowaBiala(staraPoz.x, staraPoz.y, x, y);
                    }
                    if (wybranaFigura == KROLOWA_CZARNA && ruch == 1) {
                        ok = KrolowaCzarna(staraPoz.x, staraPoz.y, x, y);
                    }
                    if (wybranaFigura == SKOCZEK_BIALY && ruch == 1) {
                        ok = SkoczekBialy(staraPoz.x, staraPoz.y, x, y);
                    }
                    if (wybranaFigura == SKOCZEK_CZARNY && ruch == 1) {
                        ok = SkoczekCzarny(staraPoz.x, staraPoz.y, x, y);
                    }
                    if (wybranaFigura == KROL_CZARNY && ruch == 1) {
                        ok = KrolCzarny(staraPoz.x, staraPoz.y, x, y);
                        if (ok == 1 && krolCzarnyRuch == 0) {
                            krolCzarnyRuch = 1;
                        }
                    }
                    if (wybranaFigura == KROL_BIALY && ruch == 1) {
                        ok = KrolBialy(staraPoz.x, staraPoz.y, x, y);
                        if (ok == 1 && krolBialyRuch == 0) {
                            krolBialyRuch = 1;
                        }
                    }
                    if (ok == 1) {
                        int nr = plansza[y][x];
                        plansza[y][x] = wybranaFigura;
                        if (y == 0 && wybranaFigura == PIONEK_BIALY) {
                            promocjaBialy = 1;
                            promocjaB.x = x;
                            promocjaB.y = y;
                            plansza[y][x] = 0;
                        }
                        if (y == 7 && wybranaFigura == PIONEK_CZARNY) {
                            promocjaCzarny = 1;
                            promocjaC.x = x;
                            promocjaC.y = y;
                            plansza[y][x] = 0;
                        }
                        if (tura == 0) { // bia³y ruszy³ siê i teraz czarny
                            if (szachBialy == 1) {
                                krolBialy = PozycjaKrolBialy(plansza);
                                int s = KrolBialySzachCheck(plansza, krolBialy.x, krolBialy.y);
                                if (s == 0) {
                                    plansza[staraPoz.y][staraPoz.x] = wybranaFigura;
                                    plansza[y][x] = nr;
                                }
                                else {
                                    szachBialy = 0;
                                    krolCzarny = PozycjaKrolCzarny(plansza);
                                    int szach = KrolCzarnySzachCheck(plansza, krolCzarny.x, krolCzarny.y);
                                    if (szach == 0) {
                                        szachCzarny = 1;
                                        wynik = CzarnyPatMat(); // mat
                                    }
                                    
                                    tura = 1;
                                    if (CzarnyPatMat() != 0 && szachCzarny == 0){ // pat
                                        wynik = 3;
                                    }
                                    if (!isPlayerVsPlayer && wynik == 0) {
                                        makeAiMove(plansza);
                                        
                                        if (BialyPatMat() != 0 && szachBialy == 0) { // pat
                                            wynik = 3;
                                        }
                                        tura = 0;
                                    }
                                }
                            }
                            else {
                                krolBialy = PozycjaKrolBialy(plansza);
                                int sa = KrolBialySzachCheck(plansza, krolBialy.x, krolBialy.y);
                                if (sa == 0) {
                                    plansza[staraPoz.y][staraPoz.x] = wybranaFigura;
                                    plansza[y][x] = nr;
                                }
                                else {
                                    krolCzarny = PozycjaKrolCzarny(plansza);
                                    int szach = KrolCzarnySzachCheck(plansza, krolCzarny.x, krolCzarny.y);
                                    if (szach == 0) {
                                        szachCzarny = 1;
                                        // tutaj dodac sprawdzenie mata
                                        // sprawdzic kazdy mozliwy ruch czarnych i czy po jego wykonaniu nadal wystepuje szach
                                        // jezeli chociaz jeden ruch jest poprawny i niweluje szach to NIE MA MATA
                                        // je¿eli wszystkie ruchy s¹ nie poprawne to MAT
                                        wynik = CzarnyPatMat(); // mat
                                    }
                                    tura = 1;
                                    if (CzarnyPatMat() != 0 && szachCzarny == 0) { // pat
                                        wynik = 3;
                                    }
                                    if (!isPlayerVsPlayer && wynik == 0) {
                                        makeAiMove(plansza);

                                        if (BialyPatMat() != 0 && szachBialy == 0) { // pat
                                            wynik = 3;
                                        }
                                        tura = 0;
                                    }
                                }
                            }
                        }
                        else { // czarny ruszy³ siê i teraz bia³y
                            if (szachCzarny == 1) {
                                krolCzarny = PozycjaKrolCzarny(plansza);
                                int s = KrolCzarnySzachCheck(plansza, krolCzarny.x, krolCzarny.y);
                                if (s == 0) {
                                    plansza[staraPoz.y][staraPoz.x] = wybranaFigura;
                                    plansza[y][x] = nr;
                                }
                                else {
                                    szachCzarny = 0;
                                    krolBialy = PozycjaKrolBialy(plansza);
                                    int szach = KrolBialySzachCheck(plansza, krolBialy.x, krolBialy.y);
                                    if (szach == 0) {
                                        szachBialy = 1;
                                        wynik = BialyPatMat(); // mat
                                    }
                                    tura = 0;
                                    if (BialyPatMat() != 0 && szachBialy == 0) { // pat
                                        wynik = 3;
                                    }
                                }
                            }
                            else {
                                krolCzarny = PozycjaKrolCzarny(plansza);
                                int sa = KrolCzarnySzachCheck(plansza, krolCzarny.x, krolCzarny.y);
                                if (sa == 0) {
                                    plansza[staraPoz.y][staraPoz.x] = wybranaFigura;
                                    plansza[y][x] = nr;
                                }
                                else {
                                    krolBialy = PozycjaKrolBialy(plansza);
                                    int szach = KrolBialySzachCheck(plansza, krolBialy.x, krolBialy.y);
                                    if (szach == 0) {
                                        szachBialy = 1;
                                        wynik = BialyPatMat(); // mat
                                    }
                                    tura = 0;
                                    if (BialyPatMat() != 0 && szachBialy == 0) { // pat
                                        wynik = 3;
                                    }
                                }
                            }
                        }
                    }
                    else if (ok == 0) {
                        plansza[staraPoz.y][staraPoz.x] = wybranaFigura;
                    }
                    ruch = 0;
                }
            }
        }
        // kod odpowiedzialny za wyœwietlanie 
        okno.clear();
        wyswietlPlansze(okno);
        if (promocjaBialy == 1) {
            PromocjaBIALY.setPosition(promocjaB.x * rozmiar, promocjaB.y * rozmiar);
            okno.draw(PromocjaBIALY);
        }
        if (promocjaCzarny == 1) {
            PromocjaCZARNY.setPosition(promocjaC.x * rozmiar, promocjaC.y * rozmiar);
            okno.draw(PromocjaCZARNY);
        }
        if (ruch == 1) {
            Ruch.setPosition(pos.x - dx, pos.y - dy);
            okno.draw(Ruch);
        }
        // Drukowanie wszystkich figur na planszy
        for (int i = 0; i < DLUGOSC; i++) {
            for (int j = 0; j < DLUGOSC; j++) {
                if (plansza[i][j] != 0) {
                    if (plansza[i][j] == PIONEK_CZARNY) {
                        PionC.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(PionC);
                    }
                    if (plansza[i][j] == PIONEK_BIALY) {
                        PionB.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(PionB);
                    }
                    if (plansza[i][j] == WIEZA_CZARNA) {
                        WiezaC.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(WiezaC);
                    }
                    if (plansza[i][j] == WIEZA_BIALA) {
                        WiezaB.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(WiezaB);
                    }
                    if (plansza[i][j] == SKOCZEK_BIALY) {
                        SkoczekB.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(SkoczekB);
                    }
                    if (plansza[i][j] == SKOCZEK_CZARNY) {
                        SkoczekC.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(SkoczekC);
                    }
                    if (plansza[i][j] == GONIEC_CZARNY) {
                        GoniecC.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(GoniecC);
                    }
                    if (plansza[i][j] == GONIEC_BIALY) {
                        GoniecB.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(GoniecB);
                    }
                    if (plansza[i][j] == KROLOWA_BIALA) {
                        KrolowaB.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(KrolowaB);
                    }
                    if (plansza[i][j] == KROLOWA_CZARNA) {
                        KrolowaC.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(KrolowaC);
                    }
                    if (plansza[i][j] == KROL_CZARNY) {
                        KrolC.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(KrolC);
                    }
                    if (plansza[i][j] == KROL_BIALY) {
                        KrolB.setPosition(j * rozmiar, i * rozmiar);
                        okno.draw(KrolB);
                    }
                }
            }
        }

        // Ustawianie tekstu w zale¿noœci od wyniku
        if (wynik == 1) {
            wynikTekst.setString("Wygrywaja biale");
        }
        else if (wynik == 2) {
            wynikTekst.setString("Wygrywaja czarne");
        }
        else if (wynik == 3) {
            wynikTekst.setString("     Remis");
        }

        // Rysowanie przycisków
        okno.draw(zapiszPrzycisk);
        okno.draw(poddajPrzycisk);
        okno.draw(wczytajPrzycisk);
        okno.draw(zapiszTekst);
        okno.draw(poddajTekst);
        okno.draw(wczytajTekst);

        if (wynik != 0) {
            okno.draw(wynikTekst);
            okno.draw(zagrajPonowniePrzycisk);
            okno.draw(zagrajPonownieTekst);
        }

        // Obs³uga klikniêcia przycisku "Zagraj Ponownie"
        if (wynik != 0 && e.type == Event::MouseButtonPressed && e.key.code == Mouse::Left) {
            if (pos.x >= 275 && pos.x <= 525 && pos.y >= 500 && pos.y <= 530) {
                RestartGry();
            }
        }

        okno.display();
    }
    return 0;
}