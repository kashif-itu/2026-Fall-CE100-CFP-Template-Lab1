#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

// catch.hpp has already emitted its own main(), so the student driver is pulled
// in here under a different name. That lets the quiz run in-process exactly as
// it was written, with no changes required to main.cpp.
#define main student_main
#include "main.cpp"
#undef main

// ---------------------------------------------------------------------------
// Test harness
// ---------------------------------------------------------------------------

static const char* QUESTIONS_FILE = "questions.json";
static const char* ANSWERS_FILE   = "answers.txt";

struct RunResult {
    int         exitCode;
    std::string out;
    std::string err;
};

static void writeFile(const char* path, const std::string& text) {
    std::ofstream f(path);
    f << text;
    f.close();
}

static std::string readFile(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return std::string("<<missing>>");
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool fileExists(const char* path) {
    std::ifstream f(path);
    return f.is_open();
}

static void clearFixtures() {
    std::remove(QUESTIONS_FILE);
    std::remove(ANSWERS_FILE);
}

// Runs the quiz once with keystrokes on stdin, capturing stdout and stderr.
static RunResult runQuiz(const std::string& keystrokes) {
    // The padding keeps a mis-counted question loop from blocking on an empty
    // stream; a wrong answer count is then caught by the assertions instead.
    std::istringstream in(keystrokes + "\nA\nA\nA\nA\nA\n");
    std::ostringstream out;
    std::ostringstream err;

    std::streambuf* oldIn  = std::cin.rdbuf(in.rdbuf());
    std::streambuf* oldOut = std::cout.rdbuf(out.rdbuf());
    std::streambuf* oldErr = std::cerr.rdbuf(err.rdbuf());

    RunResult r;
    r.exitCode = student_main();

    std::cin.clear();
    std::cin.rdbuf(oldIn);
    std::cout.rdbuf(oldOut);
    std::cerr.rdbuf(oldErr);

    r.out = out.str();
    r.err = err.str();
    return r;
}

static bool contains(const std::string& hay, const std::string& needle) {
    return hay.find(needle) != std::string::npos;
}

static int countOf(const std::string& hay, const std::string& needle) {
    int    n   = 0;
    size_t pos = hay.find(needle);
    while (pos != std::string::npos) {
        ++n;
        pos = hay.find(needle, pos + needle.size());
    }
    return n;
}

// True when every needle appears, and appears in the given order.
static bool inOrder(const std::string& hay, const std::string* needles, int count) {
    size_t from = 0;
    for (int i = 0; i < count; ++i) {
        size_t pos = hay.find(needles[i], from);
        if (pos == std::string::npos) return false;
        from = pos + needles[i].size();
    }
    return true;
}

// --- question banks --------------------------------------------------------

static std::string bankOne() {
    return "{\n"
           "  \"questions\": [\n"
           "    { \"statement\": \"Which structure follows First In First Out?\",\n"
           "      \"options\": [\"Stack\", \"Queue\", \"Tree\", \"Graph\"] }\n"
           "  ]\n"
           "}\n";
}

static std::string bankThree() {
    return "{\n"
           "  \"questions\": [\n"
           "    { \"statement\": \"Which structure follows First In First Out?\",\n"
           "      \"options\": [\"Stack\", \"Queue\", \"Tree\", \"Graph\"] },\n"
           "    { \"statement\": \"What is the cost of binary search?\",\n"
           "      \"options\": [\"O(n)\", \"O(log n)\", \"O(n log n)\", \"O(1)\"] },\n"
           "    { \"statement\": \"Which traversal visits the root first?\",\n"
           "      \"options\": [\"Inorder\", \"Preorder\", \"Postorder\", \"Levelorder\"] }\n"
           "  ]\n"
           "}\n";
}

static std::string bankFive() {
    return "{\n"
           "  \"questions\": [\n"
           "    { \"statement\": \"Which one is a linear data structure?\",\n"
           "      \"options\": [\"Alpha\", \"Bravo\", \"Charlie\", \"Delta\", \"Echo\"] }\n"
           "  ]\n"
           "}\n";
}

static const std::string PROMPT = "Your answer (A/B/C/D/E): ";

// ===========================================================================
// Task 1 - Load the question bank from questions.json
// ===========================================================================

TEST_CASE("Task 1 - a valid questions.json is read and the quiz finishes cleanly") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankThree());

    RunResult r = runQuiz("A\nB\nC\n");

    CHECK(r.exitCode == 0);
    CHECK(fileExists(ANSWERS_FILE) == true);
    CHECK(r.err.empty() == true);

    clearFixtures();
}

TEST_CASE("Task 1 - a missing questions.json is reported and stops the program") {
    clearFixtures();   // deliberately no questions.json on disk

    RunResult r = runQuiz("A\n");

    CHECK(r.exitCode == 1);
    CHECK(r.err.empty() == false);
    CHECK(contains(r.err, "Failed to open") == true);
    CHECK(fileExists(ANSWERS_FILE) == false);

    clearFixtures();
}

TEST_CASE("Task 1 - every question in the bank is asked exactly once") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankThree());

    RunResult r = runQuiz("A\nA\nA\n");

    CHECK(contains(r.out, "Question 1:") == true);
    CHECK(contains(r.out, "Question 2:") == true);
    CHECK(contains(r.out, "Question 3:") == true);
    CHECK(contains(r.out, "Question 4:") == false);
    CHECK(countOf(r.out, PROMPT) == 3);

    clearFixtures();
}

// ===========================================================================
// Task 2 - Display each question with lettered options
// ===========================================================================

TEST_CASE("Task 2 - each statement is printed against its question number") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankThree());

    RunResult r = runQuiz("A\nA\nA\n");

    std::string order[6] = { "Question 1:", "Which structure follows First In First Out?",
                             "Question 2:", "What is the cost of binary search?",
                             "Question 3:", "Which traversal visits the root first?" };
    CHECK(inOrder(r.out, order, 6) == true);

    clearFixtures();
}

TEST_CASE("Task 2 - options are labelled A, B, C, D in the order they are stored") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankOne());

    RunResult r = runQuiz("A\n");

    std::string order[8] = { "A. ", "Stack", "B. ", "Queue",
                             "C. ", "Tree",  "D. ", "Graph" };
    CHECK(inOrder(r.out, order, 8) == true);
    CHECK(contains(r.out, "E. ") == false);

    clearFixtures();
}

TEST_CASE("Task 2 - a five option question is labelled all the way to E") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankFive());

    RunResult r = runQuiz("E\n");

    std::string order[10] = { "A. ", "Alpha", "B. ", "Bravo", "C. ", "Charlie",
                              "D. ", "Delta", "E. ", "Echo" };
    CHECK(inOrder(r.out, order, 10) == true);
    CHECK(contains(r.out, "F. ") == false);

    clearFixtures();
}

// ===========================================================================
// Task 3 - Prompt for an answer and validate it
// ===========================================================================

TEST_CASE("Task 3 - the prompt is shown once per question when the input is valid") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankThree());

    RunResult r = runQuiz("B\nD\nA\n");

    CHECK(countOf(r.out, PROMPT) == 3);
    CHECK(readFile(ANSWERS_FILE) == std::string("b,d,a"));

    clearFixtures();
}

TEST_CASE("Task 3 - a letter outside A to E is rejected and the prompt repeats") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankOne());

    RunResult r = runQuiz("Z\n1\nC\n");

    CHECK(countOf(r.out, PROMPT) == 3);
    CHECK(readFile(ANSWERS_FILE) == std::string("c"));
    CHECK(r.exitCode == 0);

    clearFixtures();
}

TEST_CASE("Task 3 - every letter from A to E is accepted on the first attempt") {
    const char* letters[5]  = { "A\n", "B\n", "C\n", "D\n", "E\n" };
    const char* recorded[5] = { "a",   "b",   "c",   "d",   "e"   };

    for (int i = 0; i < 5; ++i) {
        clearFixtures();
        writeFile(QUESTIONS_FILE, bankFive());

        RunResult r = runQuiz(letters[i]);

        CHECK(countOf(r.out, PROMPT) == 1);
        CHECK(readFile(ANSWERS_FILE) == std::string(recorded[i]));
    }

    clearFixtures();
}

// ===========================================================================
// Task 4 - Record the answers in answers.txt
// ===========================================================================

TEST_CASE("Task 4 - answers are stored in lower case, comma separated, in order") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankThree());

    runQuiz("A\nC\nB\n");

    std::string saved = readFile(ANSWERS_FILE);
    CHECK(saved == std::string("a,c,b"));
    CHECK(contains(saved, " ") == false);

    clearFixtures();
}

TEST_CASE("Task 4 - a single question is stored as one letter with no comma") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankOne());

    runQuiz("D\n");

    std::string saved = readFile(ANSWERS_FILE);
    CHECK(saved == std::string("d"));
    CHECK(countOf(saved, ",") == 0);

    clearFixtures();
}

TEST_CASE("Task 4 - the confirmation message follows a correctly joined answer list") {
    clearFixtures();
    writeFile(QUESTIONS_FILE, bankThree());

    RunResult r = runQuiz("D\nA\nC\n");

    std::string saved = readFile(ANSWERS_FILE);
    CHECK(saved == std::string("d,a,c"));
    CHECK(countOf(saved, ",") == 2);
    CHECK(saved.empty() == false);
    CHECK(saved[saved.size() - 1] != ',');
    CHECK(contains(r.out, "answers.txt") == true);

    clearFixtures();
}
