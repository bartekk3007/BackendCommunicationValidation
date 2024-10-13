#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream> // potrzebny do std::istringstream
#include <thread>
#include <chrono>
#include <cctype> // potrzebny do tolower
#include <stdexcept> // do std::logical_error
#include <windows.h> // tylko do debugowania, kod ma byc niezalezny od systemu operacyjnego

#include "utilities.hpp" //zawiera funkcje pomocnicze
#include "validation.hpp" // zawiera funckje do glosowania w proof of stake

#pragma region DoDebugowania
void setTextColor(int color) 
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
const int RED = 12;
const int GREEN = 10;
const int BLUE = 9;
const int YELLOW = 14;
const int WHITE = 15;
#pragma endregion DoDebugowania

bool isVoting = false;
std::string answer;
const char ZNAKNADAWCY = '?'; // tym oznaczam ze jest to nazwa nadawcy a nie tresc wiadomosci
std::string imie; //wazne dane do identyfikacji
int srodki;
int stawka;

void validateStake() // funkcja do sprawdzania czy stawka jest poprawna
{
    std::cout << "Podaj stawke do walidacji\n";
    std::cin >> stawka;
    while(stawka > srodki)
    {
        std::cerr << "Stawka nie moze byc wieksza nic posiadane srodki, podaj nowa stawke\n";
        std::cin >> stawka;
    }
    srodki -= stawka; // *PIENIADZE* zmniejszamy srodki o stawka jaka podalismy
}

bool isValidation(std::string word)
{
    if(compareCaseInsensitive(word, "waliduj") || compareCaseInsensitive(word, "validate"))
    {
        return true;
    }
    return false;
}

bool isIncomingTransfer(std::vector<std::string> wordsVector)
{
    //konwencja przelewu ?Nadawca PRZELEJ Odbiorca Kwota
    if(wordsVector.size() >= 4 && compareCaseInsensitive(wordsVector[1], "przelej") && compareCaseInsensitive(wordsVector[2], imie))
    {
        try
        {
            std::stoi(wordsVector[3]);
            return true;
        }
        catch(std::logic_error& logicException) // lapiemy jesli konwersja na liczbe sie nie udala (invalid_argument i out_of_range wyjatki)
        {

        }
    }
    return false;
}

void validateInput(std::string line)
{
    std::vector<std::string> wordsVector = stringToVector(line);
    if(wordsVector.size() >= 2) // jeœli jest wiecej niz tylko znacznik osoby
    {
        if(isValidation(wordsVector[1]))
        {
            std::cout << "Przechodzimy do walidacji\n"; // ?do testowania
            validateVoting();
        }
        else if(isIncomingTransfer(wordsVector))
        {
            setTextColor(GREEN); // tylko do debugowania
            std::cout << "Otrzymano przelew o wartosci " << std::stoi(wordsVector[3]) << '\n';
            setTextColor(WHITE); //tylko do debugowania

            srodki += std::stoi(wordsVector[3]); // *PIENIADZE* zwiekszamy swoje srodki o to co dostalismy

            setTextColor(BLUE); // tylko do debugowania
            std::cout << "Nowy stan konta po przelewie: " << srodki << '\n'; //wyswietlamy informacje po przelewie
            setTextColor(WHITE); //tylko do debugowania
        }
        else
        {
            std::string newName = wordsVector[0].substr(1); //wyciagamy imie kim byl nadawca zakodowane w wiadomosci ale bez znaku specjalnego
            std::string newMessage = line.substr(newName.size() + 2);
            if(newName != imie)
            {
                setTextColor(RED); // tylko do debugowania
                std::cout << "[" << newName << "]" << " "; //imie adresata (wszystkich tylko nie oryginalny nadawca)
                std::cout << newMessage << '\n';
                setTextColor(WHITE); //tylko do debugowania
            }
        }
    }
}

void tail_file_async(const std::string& file_path) 
{
    std::ifstream file(file_path); //Tworzymy strumien (nie trzeba uzywac metody open)
    if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku\n";
        return;
    }

    file.seekg(0, std::ios::end); // Przesuwamy wskaŸnik na koniec pliku (pomijamy ju¿ istniej¹ce linie)

    while (true) 
    {
        std::string line;
        while (std::getline(file, line)) // Sprawdzamy, czy s¹ nowe linie do wczytania
        {
            validateInput(line);
        }

        if (file.eof()) // Sprawdzamy, czy plik siê zamkn¹³ lub wyst¹pi³ b³¹d
        {
            file.clear(); // Czyœcimy flagê EOF
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Czekamy przed ponownym sprawdzeniem pliku
    }
}

bool isOutgoingTransfer(std::string line)
{
    //konwencja przelewu o schemacie ?Nadawca PRZELEJ Odbiorca Kwota
    std::vector<std::string> wordsVector = stringToVector(line);
    if(wordsVector.size() >=3 && compareCaseInsensitive(wordsVector[0], "przelej"))
    {
        try
	    {
		    int kwotaPrzelewu = std::stoi(wordsVector[2]);
            return true;
	    }
	    catch(std::logic_error& logicException) // lapiemy jesli konwersja na liczbe sie nie udala (invalid_argument i out_of_range wyjatki)
	    {
                
	    }
    }
    return false;
}

bool isOutgoingTransferInvalid(std::string line)
{
    if(isOutgoingTransfer(line))
    {
        std::vector<std::string> wordsVector = stringToVector(line);
		int kwotaPrzelewu = std::stoi(wordsVector[2]);
		if(kwotaPrzelewu > srodki)
        {
            return true;
        }
    }
    return false;
}

void readingInput() 
{
    std::ofstream file("plik.txt", std::ios::app); // Plik w trybie dopisywania (nie trzeba uzywac metody open)
    if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku\n";
        return;
    }

    std::string input;
    while (true) 
    {
        setTextColor(BLUE); // tylko do debugowania
        std::cout << imie << ", stan konta: " << srodki << ", stawka: " << stawka << '\n'; //wyswietlamy informacje o osobie
        setTextColor(WHITE); //tylko do debugowania
        std::getline(std::cin, input);

        std::vector<std::string> wordsVector = stringToVector(input);
        if(isVoting)
        {
            answer = input;
            isVoting = false;
        }
        else if(wordsVector.size() >= 1 && isValidation(wordsVector[0]))
        {
            std::cout << "Odczytano ze ma byc walidacja\n";
            clearFile("imiona.txt");
            clearFile("glosy.txt");
            file << ZNAKNADAWCY << imie << " "; //zapisujemy imie danej osoby poprzedzone znakiem zapytania dla ulatwienia odczytywania
            file << input << std::endl; // Zapisujemy dane do pliku (wa¿ne trzeba u¿yæ std::endl by wyczyœciæ bufor)
            std::cout << "Do pliku zapisano - " << input << '\n';
        }
        else if(isOutgoingTransferInvalid(input))
        {
            std::cout << "Nie masz wystarczajacych srodkow pienieznych na przelew\n";
        }
        else if(isOutgoingTransfer(input))
        {
            setTextColor(GREEN); // tylko do debugowania
            file << ZNAKNADAWCY << imie << " "; //zapisujemy imie danej osoby poprzedzone znakiem zapytania dla ulatwienia odczytywania
            file << input << std::endl; // Zapisujemy dane do pliku (wa¿ne trzeba u¿yæ std::endl by wyczyœciæ bufor)
            std::cout << "Do pliku zapisano - " << input << '\n';
            setTextColor(WHITE); // tylko do debugowania
            srodki -= std::stoi(wordsVector[2]); // *PIENIADZE* odejmujemy srodki ktore przelewamy
        }
        else
        {
            file << ZNAKNADAWCY << imie << " "; //zapisujemy imie danej osoby poprzedzone znakiem zapytania dla ulatwienia odczytywania
            file << input << std::endl; // Zapisujemy dane do pliku (wa¿ne trzeba u¿yæ std::endl by wyczyœciæ bufor)
            std::cout << "Do pliku zapisano - " << input << '\n';
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Czekamy przed ponownym sprawdzeniem pliku
    }
    file.close(); // Zamykanie pliku
}

int main() 
{
    std::cout << "Podaj swoje imie\n";
    std::cin >> imie; // imie jest znane przed utworzeniem watku wiec mozna go uzywac
    std::cout << "Podaj swoje srodki\n";
    std::cin >> srodki;
    validateStake();

    std::thread file_thread(tail_file_async, "plik.txt");
    readingInput();
    file_thread.join();

    return 0;
}