#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <utility>
#include <vector>
#include <set>
#include <stack>
#include <unordered_map>

#define errorCode 100500;

using namespace std;
using namespace chrono;



enum class State {
    T,
    F,
    NotChecked
};

struct DecisionItem {
    int literal;
    State value;
};

// Насыщение структурок данными из файлика
void
makeCNF(ifstream &ifstream, int &countOfLiterals, int &countOfClauses, vector<vector<int>> &clauses,
        vector<State> &values) {

    string fileLine;

//    До конца файла
    while (getline(ifstream, fileLine) && fileLine != "0") {
//        Пропуск первых строк с вводными данными
        if (fileLine.empty() || fileLine.at(0) == 'c' || fileLine.at(0) == '%') {
            continue;
        }

//        Обработка инфы по файлу (строки 23 в ханое)
        if (fileLine.at(0) == 'p') {
            string trash;
            stringstream infoLine(fileLine);
//            Формат строки: p >> cnf >> кол-во литералов (различных значений) >> кол-во клауз (осмысленных строк)
            infoLine >> trash >> trash >> countOfLiterals >> countOfClauses;
            continue;
        }

//        Составляем клаузу из строки
        stringstream infoLine(fileLine);
        vector<int> tempClause;
        int literal;

        while (infoLine >> literal && literal != 0) {
            tempClause.push_back(literal);
        }
//

        clauses.push_back(tempClause);
    }

//    Инициализируем термы
    values = vector<State>(countOfLiterals + 1, State::NotChecked);
}

// Получить значение по литералу
State getLiteralValue(int literal, vector<State> &values) {
    int index = abs(literal) - 1;

    if (literal > 0 || values.at(index) == State::NotChecked) {
        return values.at(index);
    }

    return values.at(index) == State::T ? State::F : State::T;
}

// Проверка клаузы на заполненность помеченными как TRUE литералами
bool isFilledClause(int index, vector<vector<int>> clauses, vector<State> &values) {
    for (int &it : clauses.at(index)) {
        if (getLiteralValue(it, values) == State::T) {
            return true;
        }
    }

    return false;
}

// Проверка на единичность/унарность клаузы (только один неопределенный литерал)
bool isUnaryClause(int index, vector<vector<int>> clauses, vector<State> &values) {
    bool onlyOneUndefined = false;

    for (int &it : clauses.at(index)) {
        if (getLiteralValue(it, values) == State::NotChecked) {
//            Первый раз установится TRUE, при последующем сразу выйдем
            onlyOneUndefined = !onlyOneUndefined;
            if (!onlyOneUndefined) {
                break;
            }
        }
    }

    return onlyOneUndefined;
}

// Получить литерал унарной клаузы
int getUnaryLiteral(int index, vector<vector<int>> clauses, vector<State> &values) {
    for (int &it : clauses.at(index)) {
        if (getLiteralValue(it, values) == State::NotChecked) {
            return it;
        }
    }
//    Тут мб исключение кидать нужно...
    return -100500;
}

// Проверка клаузы на пустоту
bool isEmptyClause(int index, vector<vector<int>> clauses, vector<State> &values) {
    for (int &it : clauses.at(index)) {
        if (getLiteralValue(it, values) != State::F) {
            return false;
        }
    }

    return true;
}

// Проверка на наличие пустых клауз в векторе
bool hasEmptyClauses(const vector<vector<int>> &clauses, vector<State> &values) {
    for (int i = 0; i < clauses.size(); i++) {
        if (isEmptyClause(i, clauses, values)) {
            return true;
        }
    }

    return false;
}

// Поиск литерала для распространения. Ищем самый часто встречающийся (не помеченный при этом) среди всех
int getLiteral(vector<vector<int>> clauses, vector<State> &values, int countOfLiterals) {
    unordered_map<int, int> literalCountOfEntriesMap;

//    Шото задать размер и значения при иницализации сложно, + костыль
    for (int i = 0; i < countOfLiterals; i++)
        literalCountOfEntriesMap[i + 1] = 0;

//    По всем незаполненным клаузам
    for (int i = 0; i < clauses.size(); i++) {
        if (!isFilledClause(i, clauses, values)) {
//            И по всем ее литералам
            for (int &j : clauses.at(i)) {
                int literal = abs(j);

//                Коллекционируем только непомеченные
                if (values.at(literal - 1) == State::NotChecked) {
                    literalCountOfEntriesMap.at(literal)++;
                }
            }
        }
    }

//    Инит значение
    int literal = -100500;

//    Обойдем все в мапке и найдем максимум
    for (auto it = literalCountOfEntriesMap.begin(); it != literalCountOfEntriesMap.end(); it++) {
        if (literal == -100500 || it->second > literalCountOfEntriesMap.at(literal)) {
            literal = it->first;
        }
    }

    return literal;
}

// Этап распространения переменной
void makePropagation(stack<int> &assignments, const vector<vector<int>> &clauses, vector<State> &values) {
//    Флажок, что найдеа клаза с единственный литералом
    bool unaryClauseFounded = true;

    while (unaryClauseFounded) {
        unaryClauseFounded = false;

        for (int i = 0; i < clauses.size(); i++) {
//            Действуем, если не заполнен и унарный при этом
            if (!isFilledClause(i, clauses, values) && isUnaryClause(i, clauses, values)) {
                int literal = getUnaryLiteral(i, clauses, values);
                State value = literal > 0 ? State::T : State::F;

                values.at(abs(literal) - 1) = value;
                assignments.push(literal);
                unaryClauseFounded = true;
            }
        }
    }
}

// Этап отката (когда осознали, что шли по ложному пути )
void makeRollBack(stack<int> &assignments, stack<DecisionItem> &decisions, const vector<vector<int>> &clauses,
                  vector<State> &values) {
    do {
//        Обнуляем присваивания на предыдущем распространении
        while (assignments.top() != decisions.top().literal) {
            values.at(abs(assignments.top()) - 1) = State::NotChecked;
            assignments.pop();
        }

        DecisionItem &decision = decisions.top();

//        Если мы заходили с TRUE, то теперь установим значение в FALSE
        if (decision.value == State::T) {
            decision.value = State::F;
            values.at(abs(decision.literal) - 1) = State::F;
        } else {
//            Иначе мы уже проводили манипуляции с данным элементом и откатываться стоит еще чуть выше
            assignments.pop();
            decisions.pop();
            values.at(abs(decision.literal) - 1) = State::NotChecked;

            if (!decisions.empty()) {
                makeRollBack(assignments, decisions, clauses, values);
            }
        }
    } while (!decisions.empty());
}

// Флажок, что решение найдено (нет ни одной незаполненной клаузы)
bool isSolved(const vector<vector<int>> &clauses, vector<State> &values) {
    for (int i = 0; i < clauses.size(); i++) {
        if (!isFilledClause(i, clauses, values)) {
            return false;
        }
    }

    return true;
}

//  Этап распространение
void makeDecision(stack<int> &assignments, stack<DecisionItem> &decisions, vector<vector<int>> clauses,
                  vector<State> &values,
                  int countOfLiterals) {
//    Получаем литерал к распространению
    int literal = getLiteral(std::move(clauses), values, countOfLiterals);
//    Сначала попробуем его как TRUE (в makeRollback при случае фиаско изменим на False)
    decisions.push({literal, State::T});
    assignments.push(literal);
    values.at(literal - 1) = State::T;
}

// Сам алгоритм
bool DPLLSolve(int countOfLiterals, const vector<vector<int>> &clauses, vector<State> &values) {
    stack<int> assignments;
    stack<DecisionItem> decisions;


    while (true) {
//        Этап распространения переменной
        makePropagation(assignments, clauses, values);

//        Проверка на наличие незаполненных клауз
        if (hasEmptyClauses(clauses, values)) {
//            Если таковые имеются, пора откатиться
            makeRollBack(assignments, decisions, clauses, values);

//            А если откатываться некуда, то пиши пропало
            if (decisions.empty()) {
                return false;
            }
            continue;
        }

//        Если решение найдено (все клаузы заполнены)
        if (isSolved(clauses, values)) {
            return true;
        }

//        Произвести распространение литералов
        makeDecision(assignments, decisions, clauses, values, countOfLiterals);
    }
}

int main() {
// 87 sec best
//    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/hanoi/hanoi4.cnf";
//    5 milli best
    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/sat20/uf20-01.cnf";
//  53 milli best
//    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/unsat50/uuf50-01.cnf";
// 413 milli best
//    string filename = "/home/vladimir/MSU/kamkin/hw/lab3/data/unsat100/uuf100-01.cnf";

//    Почитаем файлик DIMACS
    ifstream fin(filename);
    if (!fin) {
        cout << "Ошибка отрытия файлы с КНФ '" << filename << "'" << endl;
        return -1;
    }

//    Кол-во литералов
    int countOfLiterals = 0;
//    Аналогично клауз
    int countOfClauses = 0;
//    Сами клаузы
    vector<vector<int>> clauses;
//    Значения термов
    vector<State> values;

//    Читаем-с из файлика
    makeCNF(fin, countOfLiterals, countOfClauses, clauses, values);
    fin.close();

//    Время до алгоритма
    milliseconds startTS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    cout << (DPLLSolve(countOfLiterals, clauses, values) ? "SAT" : "UNSAT") << endl << endl << endl << endl;

//    Время после
    milliseconds endTS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    cout << "DPLL: " << (chrono::duration_cast<milliseconds>(endTS - startTS).count()) << " ms" << endl;
}