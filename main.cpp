
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"  // Make sure to include the correct path to json.hpp
using json = nlohmann::json;
using namespace std;
int main() {
    // Reading data
    ifstream jsonFile("questions.json");
    if (!jsonFile.is_open()) {
        cerr << "Failed to open JSON file." << endl;
        return 1;
    }
    json inputFile;
    jsonFile >> inputFile;
    jsonFile.close();

    // Collecting Answers
    string ans_list ="";
    int questionNumber = 1;
    json mcqs = inputFile["questions"];
    for (const auto& mcq : mcqs) {
        cout << "Question " << questionNumber << ": " << mcq["statement"] << endl;
        for (size_t i = 0; i < mcq["options"].size(); ++i) {
            cout << "    " << static_cast<char>('A' + i) << ". " << mcq["options"][i] << endl;
        }

        char userAnswer;
        do {
            cout << "Your answer (A/B/C/D/E): ";
            cin >> userAnswer;
        }while(userAnswer<'A' || userAnswer >'E');
        userAnswer = tolower(userAnswer);
        if (questionNumber==1)
            ans_list = userAnswer;
        else
            ans_list = ans_list+","+ userAnswer;
        ++questionNumber;
    }
    // Writing Answers
    ofstream answersFile("answers.txt");
    if (!answersFile.is_open()) {
        cerr << "Failed to create user answers file." << endl;
        return 1;
    }
    answersFile << ans_list;
    answersFile.close();

    cout << "User answers have been written to answers.txt." << endl;

    return 0;
}
