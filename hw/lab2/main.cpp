/*
 * Copyright 2021 ISP RAS (http://www.ispras.ru)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

#include <iostream>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include<string>
#include<cmath>
#include<stack>
#include<functional>

#include "bdd.h"
#include "formula.h"


using namespace model::bdd;
using namespace model::logic;
using namespace std;

// Получаем хеш для нашего объекта формула для добавления в сеты
template<>
struct std::hash<Formula> {
    int operator()(Formula const &formula) const {
        hash<string> hashFunc;
        return hashFunc(formula.toString());
    }
};

ostream &operator<<(ostream &out, unordered_set<Formula> &formulas) {
    for (const auto &formula : formulas) {
        out << formula << ", ";
    }
    return out;
}

bool IsOperator(const Formula &formula);

bool getResultOfRow(bool left, bool right, Formula::Kind boolOperator);

std::vector<Node> &
getRootsRecur(Formula &formula1, vector<int> &query, vector<unordered_map<Formula, Formula>> &variablesCombinations,
              vector<Node> &roots, unordered_map<Formula, Formula> &neededs, vector<int> &mainQuery, int level);

bool canAdd(const Formula &formula, const unordered_map<Formula, Formula> &ignored) {
    for (auto &i : ignored) {
        if (formula.kind() == i.first.kind() && formula.var() == i.first.var()) {
            return false;
        }
    }

    return true;
}

void getVariables(const Formula &formula, unordered_set<Formula> &setOfVariables,
                  const unordered_map<Formula, Formula> &ignored) {
    if (formula.var() != -1 && canAdd(formula, ignored)) {
        setOfVariables.insert(formula);
    }

    if (&formula.lhs() != nullptr) {
        getVariables(formula.lhs(), setOfVariables, ignored);
    }

    if (&formula.rhs() != nullptr) {
        getVariables(formula.rhs(), setOfVariables, ignored);
    }
}

unordered_map<Formula, Formula> getTrueSet(unordered_set<Formula> &variables, int index) {
    unordered_map<Formula, Formula> trueSet;
    auto k = variables.begin();
    while (index != 0) {
        if (index % 2 == 1) {
            trueSet.insert({*k, Formula::T});
        } else {
            trueSet.insert({*k, Formula::F});
        }
        index /= 2;
        k++;
    }

    while (k != variables.end()) {
        trueSet.insert({*k, Formula::F});
        k++;
    }
    return trueSet;
}

vector<unordered_map<Formula, Formula> > makeVariablesCombinations(unordered_set<Formula> variables) {
    int vectorCombinationsSize = pow(2, variables.size());
    vector<unordered_map<Formula, Formula>> combinations;
    for (int i = 0; i < vectorCombinationsSize; i++) {
        unordered_map<Formula, Formula> trueSet = getTrueSet(variables, i);
        combinations.push_back(trueSet);
    }
    return combinations;
}

void
getResultForCurrentRow(unordered_map<Formula, Formula> currentRow, Formula &formula, vector<Formula> &stackFormulas) {
    if (IsOperator(formula)) {
        Formula left = formula.lhs();
        Formula right = formula.rhs();
        bool modifiedRight = false;
        bool modifiedLeft = false;

        if (IsOperator(left)) {
            getResultForCurrentRow(currentRow, left, stackFormulas);
            modifiedLeft = true;
        }
        if (left.kind() == Formula::NOT) {
            getResultForCurrentRow(currentRow, left, stackFormulas);
            modifiedLeft = true;
        }
        Formula leftValue = modifiedLeft ? stackFormulas.at(stackFormulas.size() - 1) : currentRow.at(left);

        if (IsOperator(right)) {
            getResultForCurrentRow(currentRow, right, stackFormulas);
            modifiedRight = true;
        }
        if (right.kind() == Formula::NOT) {
            getResultForCurrentRow(currentRow, right, stackFormulas);
            modifiedRight = true;
        }
        Formula rightValue = modifiedRight ? stackFormulas.at(stackFormulas.size() - 1) : currentRow.at(right);

        bool leftBool = leftValue.kind() == Formula::TRUE;
        bool rightBool = rightValue.kind() == Formula::TRUE;

        bool result = getResultOfRow(leftBool, rightBool, formula.kind());

        stackFormulas.push_back(result ? Formula::T : Formula::F);
        return;
    }

    if (formula.kind() == Formula::NOT) {
        Formula left = formula.lhs();
        getResultForCurrentRow(currentRow, left, stackFormulas);
        stackFormulas.emplace_back(stackFormulas.at(stackFormulas.size() - 1).kind() == Formula::T.kind() ? Formula::F : Formula::T);
        return;
    }

    stackFormulas.emplace_back(currentRow.at(formula));
}

bool getResultOfRow(bool left, bool right, Formula::Kind boolOperator) {
    switch (boolOperator) {
        case Formula::AND:
            return left && right;
        case Formula::OR:
            return left || right;
        case Formula::EQ:
            return left == right;
        case Formula::IMPL:
            return !left || right;
        case Formula::XOR:
            return left != right;
        default:
            return false;
    }
}

void calculateFormula(Formula formula, const vector<unordered_map<Formula, Formula> > &variablesCombinations,
                      vector<int> &resultVector) {
    vector<Formula> stackFormulas;
    for (const auto &currentRow : variablesCombinations) {
        vector<Formula> edited;
        getResultForCurrentRow(currentRow, formula, stackFormulas);

        resultVector.push_back(stackFormulas.at(stackFormulas.size() - 1).kind() == Formula::TRUE ? 1 : 0);
    }
}

bool IsOperator(const Formula &formula) {
    return formula.kind() == Formula::AND || formula.kind() == Formula::EQ || formula.kind() == Formula::IMPL ||
           formula.kind() == Formula::OR || formula.kind() == Formula::XOR;
}

Formula getNeededVar(unordered_map<Formula, Formula> &map, int &at) {
    for (auto &it : map) {
        if (it.first.var() == at) {
            return it.first;
        }
    }

    return Formula::F;
}

void insertOrUpdateNeeded(unordered_map<Formula, Formula> &neededs, const Formula &needed, const Formula &site) {
    for (auto &it : neededs) {
        if (it.first.kind() == needed.kind() && it.first.var() == needed.var()) {
            neededs.erase(it.first);
        }
    }
    neededs.insert({needed, site});
}

void makePrint(unordered_map<Formula, Formula> &neededs, vector<int> &mainQuery, const Formula &site, Node nodeRoot,
               Node node, const string &value) {
    unordered_map<Formula, Formula> temp;
    for (auto &it : neededs) {
        temp.insert(it);
    }

    int needSize = neededs.size();
    int index = 0;
    while (index < needSize) {
        for (auto &it : temp) {
            for (int &i : mainQuery) {
                Formula need = getNeededVar(temp, i);
                if (need.kind() != Formula::F.kind()) {
                    if (it.first.kind() == need.kind() && it.first.var() == need.var()) {
                        if (index == needSize - 1) {
                            if (it.second.kind() == Formula::T.kind()) {
                                nodeRoot.high = &node;
                                cout << "[x" << it.first.var() << "] ---> high [" << value << "]" << endl;
                            } else {
                                nodeRoot.low = &node;
                                cout << "[x" << it.first.var() << "] ---> low [" << value << "]" << endl;
                            }
                        } else {
                            if (it.second.kind() == Formula::T.kind()) {
                                nodeRoot.high = &node;
                                cout << "[x" << need.var() << "] ---> high ";
                            } else {
                                nodeRoot.low = &node;
                                cout << "[x" << need.var() << "] ---> low ";
                            }
                        }
                        index++;
                    }
                }
            }
        }
    }
}

void addRootLinks(vector<Node> &roots, vector<int> &resultVectorTemp, const Formula &site, vector<int> &query,
                  unordered_map<Formula, Formula> map, unordered_map<Formula, Formula> &neededs, Formula &formula1,
                  vector<unordered_map<Formula, Formula>> &variablesCombinations, vector<int> &mainQuery, int level) {
    bool isOne = true;
    bool isZero = true;
    for (int j : resultVectorTemp) {
        if (j == 1) {
            isZero = false;
        }
        if (j == 0) {
            isOne = false;
        }
    }

    Node &nodeRoot = roots.at(roots.size() - 1);
    if (isOne) {
        Node node = Node(10000000, nullptr, nullptr);
        makePrint(neededs, mainQuery, site, nodeRoot, node, "1");
    }
    if (isZero) {
        Node node = Node(-10000000, nullptr, nullptr);
        makePrint(neededs, mainQuery, site, nodeRoot, node, "0");
    }
    if (!isZero && !isOne) {
        Formula need = getNeededVar(map, query.at(level));
        Node newRoot = Node(need.var(), nullptr, nullptr);
        roots.push_back(newRoot);
        insertOrUpdateNeeded(neededs, need, Formula::T);

        getRootsRecur(formula1, query, variablesCombinations, roots, neededs, mainQuery, level + 1);
    }
}

void
handleRootSite(Formula &formula1, vector<Node> &roots, unordered_map<Formula, Formula> &neededs,
               unordered_set<Formula> &variablesTemp, const Formula &site, vector<int> &query,
               unordered_map<Formula, Formula> &map, vector<int> &mainQuery, int level) {
    vector<unordered_map<Formula, Formula> > variablesCombinationsTemp = makeVariablesCombinations(variablesTemp);
    for (auto &it : variablesCombinationsTemp) {
        for (auto &needed : neededs) {
            it.insert({needed.first, needed.second});
        }
    }

    vector<int> resultVectorTemp = vector<int>();
    calculateFormula(formula1, variablesCombinationsTemp, resultVectorTemp);
    addRootLinks(roots, resultVectorTemp, site, query, map, neededs, formula1, variablesCombinationsTemp, mainQuery,
                 level);
    neededs.erase(neededs.cbegin());
}

Formula getLastNeeded(unordered_map<Formula, Formula> &map, int at) {
    Formula result = Formula::T;
    int index = 0;
    for (auto &iter : map) {
        if (at == index) {
            return iter.first;
        }
        index++;
    }

    return result;
}

vector<Node> &
getRootsRecur(Formula &formula1, vector<int> &query, vector<unordered_map<Formula, Formula>> &variablesCombinations,
              vector<Node> &roots, unordered_map<Formula, Formula> &neededs, vector<int> &mainQuery, int level) {
    unordered_set<Formula> variablesTemp;
    getVariables(formula1, variablesTemp, neededs);
    Formula last = getLastNeeded(neededs, 0);
    insertOrUpdateNeeded(neededs, last, Formula::T);
    handleRootSite(formula1, roots, neededs, variablesTemp, Formula::T, query, variablesCombinations.at(0), mainQuery,
                   level);
    insertOrUpdateNeeded(neededs, last, Formula::F);
    handleRootSite(formula1, roots, neededs, variablesTemp, Formula::F, query, variablesCombinations.at(0), mainQuery,
                   level);
    return roots;
}


int main() {
//    Исходная формула
//    Formula formula1 = x(1) || x(2) && x(3);
//    Formula formula1 = x(1) != x(2) != x(3);
    Formula formula1 = (x(1) >> (x(2) && x(3))) >> !(x(1) || (x(2) >> x(3)));
//    Formula formula2 = (x(1) || (x(2) >> x(3)));
//    Порядок переменных (плиз индексы теже ставьте, заглушки и проверки не ставил)
    vector<int> query = vector<int>{2, 1, 3};
//    vector<int> query = vector<int>{0, 1};
    vector<int> mainQuery = query;
//    Formula formula1 = x(0) >> x(1);
//     Formula formula1 = !x(1) >> !x(0);
//     Formula formula1 = T;
//
    std::cout << "Formulae: " << std::endl;
    std::cout << formula1 << std::endl;
    Bdd bdd;

    unordered_set<Formula> variables;
    getVariables(formula1, variables, unordered_map<Formula, Formula>());

    vector<unordered_map<Formula, Formula> > variablesCombinations = makeVariablesCombinations(variables);
    vector<int> resultVector = vector<int>();
    calculateFormula(formula1, variablesCombinations, resultVector);

    cout << "Вектор функции: " << endl;
    for (int i : resultVector) {
        cout << i << ", ";
    }
    cout << endl;
    cout << endl;


    cout << "Таблица истинности: " << endl;
    cout << "|   ";
    if (!variables.empty()) {
        for (int i = 0; i < variables.size(); i++) {
            cout << "x" << (i + 1) << "   |   ";
        }
        cout << "result   |" << endl;
    }
    cout << "-------------------------------------------------------------------------------------" << endl;
    for (int i = 0; i < resultVector.size(); i++) {
        cout << "|   ";
        for (auto &it : variablesCombinations.at(i)) {
            cout << (it.second.toString() == "false" ? "F" : "T") << "    |   ";
        }
        cout << resultVector.at(i) << "   |" << endl;
    }
    cout << endl;
    cout << endl;

    int level = 0;
    vector<Node> roots;
    unordered_map<Formula, Formula> neededs;
    Formula needed = getNeededVar(variablesCombinations.at(0), query.at(level));
    Node root = bdd.create(needed);
    roots.push_back(root);
    insertOrUpdateNeeded(neededs, needed, Formula::T);
    roots = getRootsRecur(formula1, query, variablesCombinations, roots, neededs, mainQuery, 1);

    return 0;
}






