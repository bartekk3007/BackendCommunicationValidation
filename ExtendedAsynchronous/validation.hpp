#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <sstream> // potrzebny do std::istringstream
#include <thread>
#include <chrono>
#include <cctype> // potrzebny do tolower
#include <stdexcept> // do std::logical_error
#include <windows.h> // tylko do debugowania, kod ma byc niezalezny od systemu operacyjnego

#include "utilities.hpp"

extern std::string imie; // deklarujemy zmienne globalne z pliku main
extern int srodki;
extern int stawka;
extern bool isVoting;
extern std::string answer;
std::map<std::string, int> namesStakesMap;
std::map<std::string, int> validatorsMap;
std::map<std::string, bool> answerersMap;
std::string seedString;
std::hash<std::string> hasher;
bool votingResult;
double stakingYield = 0.05; // zwrot procentowy to okolo 5% 

void addAnswerers(std::string line)
{
    std::vector<std::string> wordsvector = stringToVector(line);
    if(wordsvector.size() >= 2)
    {
        std::pair<std::string, bool> nameStakePair = std::make_pair(wordsvector[0], std::stoi(wordsvector[1]));
        answerersMap.insert(nameStakePair);
    }
}

void readAnswers(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) // Sprawdzamy, czy s¹ nowe linie do wczytania
    {
        addAnswerers(line);
    }
}

void writeAnswerToFile(bool boolAnswer)
{
    std::ofstream file("glosy.txt", std::ios::app);
	if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku\n";
        return;
    }
	file << imie << " " << boolAnswer << std::endl;
	file.close();
}

void stakeAndSlash(const std::map<std::string, bool>& votersResults)
{
    for(auto singleVoter : votersResults)
    {
        if(singleVoter.first == imie)
        {
            if(singleVoter.second == votingResult)
            {
                stawka = stawka + stawka * stakingYield;
                std::cout << "Prawidlowa weryfikacja, twoja stawka w wyniku stackingu to " << stawka << '\n';
            }
            else
            {
                stawka = 0;
                std::cout << "Nieprawidlowa weryfikacja, twoja stawka w wyniku slashingu to " << stawka << '\n';
            }
        }
    }
}

void vote()
{
    answer = "";
    isVoting = true;
    bool boolAnswer;
    std::cout << "Czy blok jest poprawny?\n";
    std::cout << "Jesli tak wpisz - prawda, poprawny lub tak\n";
    std::cout << "Jesli nie wpisz - falsz, niepoprawny lub nie\n";
    std::cout << "Masz 15 sekund na wpisanie odpowiedzi, domyslna wartosc to prawda\n";
    std::this_thread::sleep_for(std::chrono::seconds(15));
    if(compareCaseInsensitiveList(answer, {"tak", "prawda", "poprawny" "yes", "true", "valid", "1"}))
    {
        boolAnswer = true;
    }
    else if(compareCaseInsensitiveList(answer, {"nie", "falsz", "niepoprawny", "no", "false", "invalid", "0"}))
    {
        boolAnswer = false;
    }
    else
    {
        boolAnswer = true;
    }

    for(int i = 0; i < 5; i++)
    {
        writeAnswerToFile(boolAnswer);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    readAnswers("glosy.txt");

    std::cout << "Odpowiedzi walidatorow\n";
    for(auto answererVote : answerersMap)
    {
        std::cout << answererVote.first << " " << answererVote.second << '\n';
    }
    votingResult = countVotes(answerersMap);
    std::cout << "Wynik glosowania to " << votingResult << '\n';
    stakeAndSlash(answerersMap);
}

void checkValidators()
{
    for (const std::pair<const std::string, int>& validator : validatorsMap)
    {
        if(imie == validator.first)
        {
            vote();
        }
    }
}

void drawValidators()
{
    int validatorsSize = getValidatorsNumber(namesStakesMap.size());
    srand(hasher(seedString));
    while(validatorsMap.size() != validatorsSize)
    {
        std::pair<std::string, int> validator = getValidator(namesStakesMap);
        validatorsMap.insert(validator);
        namesStakesMap.erase(validator.first);
    }
}

void createSeedString()
{
    for (const std::pair<const std::string, int>& nameStake : namesStakesMap)
    {
        seedString += nameStake.first;
        seedString += std::to_string(nameStake.second);
    }
}

void addNames(std::string line)
{
    std::vector<std::string> wordsvector = stringToVector(line);
    if(wordsvector.size() >= 2)
    {
        std::pair<std::string, int> nameStakePair = std::make_pair(wordsvector[0], std::stoi(wordsvector[1]));
        namesStakesMap.insert(nameStakePair);
    }
}

void readingNames(const std::string& file_path) 
{
    std::ifstream file(file_path);
    if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku\n";
        return;
    }
    std::string line;
    while (std::getline(file, line)) // Sprawdzamy, czy s¹ nowe linie do wczytania
    {
        addNames(line);
    }
}

void writeNameToFile()
{
	std::ofstream file("imiona.txt", std::ios::app);
	if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku\n";
        return;
    }
	file << imie << " " << stawka << std::endl;
	file.close();
}

void validateVoting()
{
    for(int i = 0; i < 5; i++)
    {
        writeNameToFile();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    readingNames("imiona.txt");

    std::cout << "Lista osob mozliwych do wyboru\n";
    for (const std::pair<const std::string, int>& nameStake : namesStakesMap)
    {
        std::cout << nameStake.first << " " << nameStake.second << '\n';
    }
    std::cout << '\n';
    createSeedString();
    drawValidators();
    std::cout << "Lista walidatorow\n";
    for (const std::pair<const std::string, int>& validator : validatorsMap)
    {
        std::cout << validator.first << " " << validator.second << '\n';
    }
    std::cout << '\n';
    checkValidators();
    
    return;
}