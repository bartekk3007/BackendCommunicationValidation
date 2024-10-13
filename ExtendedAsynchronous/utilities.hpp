#ifndef UTILITIES_H
#define UTILITIES_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <functional>
#include <initializer_list>
#include <fstream>
#include <sstream> // potrzebny do std::istringstream
#include <thread>
#include <chrono>
#include <cctype> // potrzebny do tolower
#include <windows.h> // tylko do debugowania, kod ma byc niezalezny od systemu operacyjnego

std::vector<std::string> stringToVector(std::string line) // dzielimy string na vector który ma s³owa oddzielone spacjami
{
    std::vector<std::string> wordsVector;
	std::istringstream iss(line);
	std::string word;
	while(iss >> word) 
	{
		wordsVector.push_back(word);
	}
    return wordsVector;
}

bool compareCaseInsensitive(const std::string& str1, const std::string& str2) // porównywanie bez patrzenia na wielkosc liter
{
    if (str1.length() != str2.length())
    {
        return false;
    }
    for (int i = 0; i < str1.length(); ++i) 
    {
        if (tolower(str1[i]) != tolower(str2[i]))
        {
            return false;
        } 
    }
    return true;
}

bool compareCaseInsensitiveList(std::string str1, std::initializer_list<std::string> wordList)
{
    for(std::string word : wordList)
    {
        if(compareCaseInsensitive(str1, word))
        {
            return true;
        }
    }
    return false;
}

bool countVotes(const std::map<std::string, bool>& votersResults)
{
    int votesTrue = 0;
    int votesFalse = 0;
    for(auto singleVoter : votersResults)
    {
        if(singleVoter.second == true)
        {
            votesTrue++;
        }
        else
        {
            votesFalse++;
        }
    }

    if(votesFalse > votesTrue)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void clearFile(const std::string& file_path)
{
    std::ofstream file(file_path, std::ios::trunc);
	file.close();
}

int getValidatorsNumber(int number)
{
    if(number <= 3)
    {
        return 1;
    }
    else if(number/2 % 2 == 0)
    {
        return number/2 + 1;
    }
    else if(number/2 % 2 == 1)
    {
        return number/2;
    }
}

std::pair<std::string, int> getValidator(std::map<std::string, int>& personMap)
{
    //przygotowanie sumy
    int sum = 0;
    for (const std::pair<const std::string, int>& nameStake : personMap)
    {
        sum += nameStake.second;
    }
    int randomNumber = rand() % sum;

    // losowanie z proporcjami odpowiednimi do stawki
    int currentSum = 0;
    for (const std::pair<const std::string, int>& nameStake : personMap)
    {
        currentSum += nameStake.second;
        if(randomNumber < currentSum)
        {
            return std::make_pair(nameStake.first, nameStake.second);
        }
    }
}

#endif