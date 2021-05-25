#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <stack>
#include <string>
#include <chrono>

using namespace std;
using namespace chrono;

enum class State {
    T,
    F,
    NotChecked
};

struct Assignment {
    int literal;
    State value;
};


void insertClause(const string &fileLine, vector<vector<int>> &clauses) {
    stringstream infoLine(fileLine);
    vector<int> clause;
    int literal;

    while (infoLine >> literal && literal != 0) {
        clause.push_back(literal);
    }

    clauses.push_back(clause);
}

void
makeCNF(ifstream &ifstream, int &literalsCount, int &clausesCount, vector<vector<int>> &clauses,
        vector<State> &values) {
    literalsCount = 0;
    clausesCount = 0;

    string fileLine;

    while (getline(ifstream, fileLine) && fileLine != "0") {
//        Пропуск первых строк с вводными данными
        if (fileLine.empty() || fileLine.at(0) == 'c' || fileLine.at(0) == '%') {
            continue;
        }

//        Обработка инфы по файлу
        if (fileLine.at(0) == 'p') {
            string trash;
            stringstream infoLine(fileLine);
            infoLine >> trash >> trash >> literalsCount >> clausesCount;
            continue;
        }

        insertClause(fileLine, clauses);
    }

    values = vector<State>(literalsCount, State::NotChecked);
}

void printCNF(int literalsCount, int clausesCount, vector<vector<int>> clauses, const vector<int> &values) {
    cout << "Количество литералов: " << literalsCount << endl;
    cout << "Количество клауз: " << clausesCount << endl;
    cout << "Клаузы: " << endl;

    bool firstPrinted = false;
    for (int i = 0; i < clausesCount; ++i) {
        if (firstPrinted) {
//            cout << " AND " << endl;
            cout << " AND ";
        }

        cout << " (";

        vector<int> &currentClause = clauses.at(i);
        for (int j = 0; j < currentClause.size(); j++) {
            if (j != 0) {
                cout << " OR ";
            }

            int &currentClauseValue = currentClause.at(j);
            if (currentClauseValue < 0) {
                std::cout << "NOT x" << currentClauseValue;
            } else {
                std::cout << "x" << currentClauseValue;
            }
        }

//        cout << ")" << endl;
        cout << ")";
        firstPrinted = true;
    }
}

State getLiteralValue(int literal, vector<State> values) {
    int index = abs(literal) - 1;

    if (literal > 0 || values.at(index) == State::NotChecked) {
        return values.at(index);
    }

    return values.at(index) == State::T ? State::F : State::T;
}


bool isRemovedClause(int index, vector<vector<int>> clauses, vector<State> &values) {
    for (int i = 0; i < clauses.at(index).size(); i++) {
        if (getLiteralValue(clauses.at(index).at(i), values) == State::T) {
            return true;
        }
    }

    return false;
}

bool isUnitClause(int index, vector<vector<int>> clauses, vector<State> &values) {
    int undefinedCount = 0;

    for (int i = 0; i < clauses.at(index).size() && undefinedCount < 2; i++) {
        if (getLiteralValue(clauses.at(index).at(i), values) == State::NotChecked) {
            undefinedCount++;
        }
    }

    return undefinedCount == 1;
}

bool isEmptyClause(int index, const vector<vector<int>> &clauses, vector<State> &values) {
    for (int i = 0; i < clauses.at(index).size(); i++) {
        if (getLiteralValue(clauses.at(index).at(i), values) != State::F) {
            return false;
        }
    }

    return true;
}

bool haveEmptyClauses(const vector<vector<int>> &clauses, vector<State> &values) {
    for (int i = 0; i < clauses.size(); i++) {
        if (isEmptyClause(i, clauses, values)) {
            return true;
        }
    }

    return false;
}

int getUnitLiteral(int index, vector<vector<int>> clauses, vector<State> &values) {
    for (int i = 0; i < clauses.size(); i++) {
        if (getLiteralValue(clauses.at(index).at(i), values) == State::NotChecked) {
            return clauses.at(index).at(i);
        }
    }
    return 0;
}

void makeUnitPropagation(const stack<int> &stack, const vector<vector<int>> &clauses, vector<State> &values,
                         ::stack<int> &assignments) {
    bool founded = true;

    while (founded) {
        founded = false;

        for (int i = 0; i < clauses.size(); i++) {
            if (isRemovedClause(i, clauses, values) || !isUnitClause(i, clauses, values)) {
                continue;
            }

            int literal = getUnitLiteral(i, clauses, values);
            State value = literal > 0 ? State::T : State::F;

            values.at(abs(literal) - 1) = value;
            assignments.push(literal);
            founded = true;
        }
    }

}

void makeRollback(stack<int> &assignments, stack<Assignment> &decisions, vector<State> &values) {
    while (assignments.top() != decisions.top().literal) {
        values.at(abs(assignments.top()) - 1) = State::NotChecked;
        assignments.pop();
    }

    Assignment &decision = decisions.top();

    if (decision.value == State::F) {
        decision.value = State::T;
        values.at(abs(decision.literal) - 1) = State::T;
    } else {
        assignments.pop();
        decisions.pop();
        values.at(abs(decision.literal) - 1) = State::NotChecked;

        if (!decisions.empty()) {
            makeRollback(assignments, decisions, values);
        }
    }
}

bool hasSolve(const vector<vector<int>> &clauses, vector<State> &values) {
    for (int i = 0; i < clauses.size(); i++) {
        if (!isRemovedClause(i, clauses, values)) {
            return false;
        }
    }
    return true;
}

bool solveDPLL(int literalsCount, const vector<vector<int>> &clauses, vector<State> &values) {
    stack<int> assignments;
    stack<Assignment> decisions;

    while (true) {
        makeUnitPropagation(assignments, clauses, values, assignments);

        if (haveEmptyClauses(clauses, values)) {
            makeRollback(assignments, decisions, values);

            if (decisions.empty()) {
                return false;
            }

            continue;
        }

        if (hasSolve(clauses, values)) {
            return true;
        }

        int index = 0;
        while (index < literalsCount && getLiteralValue(index + 1, values) != State::NotChecked) {
            index++;
        }

        decisions.push({index + 1, State::F});
        assignments.push(index + 1);
        values.at(index) = State::F;
    }
}

//https://www.cs.ubc.ca/~hoos/SATLIB/benchm.html
int main() {
//    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/hanoi/hanoi4.cnf";
//    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/sat20/uf20-01.cnf";
    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/unsat50/uuf50-01.cnf";
//    string filename = "data/hanoi/hanoi4.cnf";

    ifstream fin(filename);
    if (!fin) {
        cout << "Ошибка отрытия файлы с КНФ '" << filename << "'" << endl;
        return -1;
    }

    int literalsCount;
    int clausesCount;
    vector<vector<int>> clauses;
    // значения термов
    // 1    - True
    // 0    - неопределено
    // -1   - False
    vector<State> values;

    makeCNF(fin, literalsCount, clausesCount, clauses, values);
    fin.close();

    cout << "Прочитанная КНФ:" << endl;
//    printCNF(literalsCount, clausesCount, clauses, values);

    milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    if (solveDPLL(literalsCount, clauses, values)) {
        cout << "SAT" << endl;
    } else {
        cout << "UNSAT" << endl;
    }
    milliseconds end = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    cout << endl;
    cout << endl;
    cout << endl;
    cout << "DPLL: " << (std::chrono::duration_cast<milliseconds>(end - start).count()) << " ms" << endl;

    return 0;
}