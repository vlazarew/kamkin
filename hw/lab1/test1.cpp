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

#include<string>
#include<math.h>
#include<stack>
#include<cstring>
#include<unordered_set>
#include<functional>
#include <memory>
#include "ltl.h"
#include "fsm.h"
using namespace model::ltl;
using namespace model::fsm;


std::string ftostr(const Formula& formula) {
    switch (formula.kind()) {
        case Formula::ATOM:
            return "(ATOM" + formula.prop() + ")";
        case Formula::NOT:
            return "NOT(" + ftostr(formula.lhs()) + ")";
        case Formula::AND:
            return "(" + ftostr(formula.lhs()) + "AND" + ftostr(formula.rhs()) + ")";
        case Formula::OR:
            return "(" + ftostr(formula.lhs()) + "OR" + ftostr(formula.rhs()) + ")";
        case Formula::IMPL:
            return "(" + ftostr(formula.lhs()) + "IMPL" + ftostr(formula.rhs()) + ")";
        case Formula::X:
            return "X(" + ftostr(formula.lhs()) + ")";
        case Formula::G:
            return "G(" + ftostr(formula.lhs()) + ")";
        case Formula::F:
            return "F(" + ftostr(formula.lhs()) + ")";
        case Formula::U:
            return "(" + ftostr(formula.lhs()) + "U" + ftostr(formula.rhs()) + ")";
        case Formula::R:
            return "(" + ftostr(formula.lhs()) + "R" + ftostr(formula.rhs()) + ")";
    };
    return "";
}

template<> struct std::hash<Formula> {
    std::size_t operator()(Formula const& key) const {
        std::hash<std::string> hasher;
        return hasher(ftostr(key));
    }
};

std::ostream& operator <<(std::ostream& out, std::vector<Formula>& formula_vector) {
    for (int i = 0; i < formula_vector.size() - 1; i++) {
        out << formula_vector[i] << ", ";
    }
    out << formula_vector[formula_vector.size() - 1];
    return out;
}

std::ostream& operator <<(std::ostream& out, std::unordered_set<Formula>& formula_vector) {
    for (auto it = formula_vector.begin(); it != formula_vector.end(); it++) {
        out << *it << ", ";
    }
    return out;
}

std::ostream& operator <<(std::ostream& out, std::vector<std::unordered_set<Formula>>& S) {
    for (int i = 0; i < S.size(); i++) {
        out << "[ " << S[i] << " ]" << std::endl;
    }
    return out;
}

void build_enclosure(Formula formula, std::unordered_set<Formula>& closure) {
    closure.insert(formula);
    if (formula.kind() == Formula::ATOM) {
        return;
    }
    else if (formula.kind() == Formula::X) {
        build_enclosure(formula.arg(), closure);
    }
    else if (formula.kind() == Formula::F) {
        build_enclosure(formula.lhs(), closure);
    }
    else if (formula.kind() == Formula::G) {
        build_enclosure(formula.lhs(), closure);
    }
    else if (formula.kind() == Formula::NOT) {
        build_enclosure(formula.lhs(), closure);
    }
    else {
        build_enclosure(formula.lhs(), closure);
        build_enclosure(formula.rhs(), closure);
    }
}

void get_elementary_sets(std::unordered_set<Formula> closure, bool& has_true, std::unordered_set<Formula>& sets) {
    for (auto & it: closure) {
        if (it.kind() == Formula::ATOM && it.prop() == "T") {
            has_true = true;
        }
        else if (it.kind() == Formula::X || it.kind() == Formula::ATOM) {
            sets.insert(it);
        }
    }
}


bool is_true(Formula f, std::unordered_set<Formula>& true_set) {
    if (f.kind() == Formula::AND) {
        return is_true(f.lhs(), true_set) && is_true(f.rhs(), true_set);
    }
    else if (f.kind() == Formula::OR) {
        return is_true(f.lhs(), true_set) || is_true(f.rhs(), true_set);
    }
    else if (f.kind() == Formula::NOT) {
        return !is_true(f.lhs(), true_set);
    }
    else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return true_set.find(f) != true_set.end();
    }
    else {
        return false;
    }
}

bool classic_logic(Formula f){
    if (f.kind() == Formula::AND || f.kind() == Formula::OR) {
        return classic_logic(f.lhs()) && classic_logic(f.rhs());
    }
    else if (f.kind() == Formula::NOT) {
        return classic_logic(f.lhs());
    }
    else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return true;
    }
    else {
        return false;
    }
}

std::unordered_set<Formula> get_s(std::unordered_set<Formula> true_set, std::unordered_set<Formula>& closure) {
    std::unordered_set<Formula> res;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (classic_logic(*it) && is_true(*it, true_set)) {
            res.insert(*it);
        }
    }
    return res;
}

std::unordered_set<Formula> get_until(std::unordered_set<Formula>& closure) {
    std::unordered_set<Formula> res;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (it->kind() == Formula::U) {
            res.insert(*it);
        }
    }
    return res;
}

std::unordered_set<Formula> get_X(std::unordered_set<Formula>& closure) {
    std::unordered_set<Formula> res;
    for (auto it = closure.begin(); it != closure.end(); it++) {
        if (it->kind() == Formula::X) {
            res.insert(*it);
        }
    }
    return res;
}

std::vector<std::unordered_set<Formula>> recombinate_elementary_sets(std::unordered_set<Formula> &elementary_sets, std::unordered_set<Formula>& closure, bool has_true) {
    std::vector<std::unordered_set<Formula>> S(pow(2, elementary_sets.size()));
    for (int i = 0; i < S.size(); i++) {
        std::unordered_set<Formula> true_set;
        auto k = elementary_sets.begin();
        int num = i;
        while (num != 0) {
            if (num % 2 == 1) {
                true_set.insert(*k);
            }
            else {
                true_set.insert(!(*k));
            }
            num /= 2;
            k++;
        }
        while (k != elementary_sets.end()) {
            true_set.insert(!(*k));
            k++;
        }

        S[i] = get_s(true_set, closure);
        if (has_true) {
            S[i].insert(P("T"));
        }
    }
    return S;
}

void add_until(std::unordered_set<Formula>& untils, std::vector<std::unordered_set<Formula>>& S) {
    std::vector<std::unordered_set<Formula>> to_add;
    for (auto until = untils.begin(); until != untils.end(); until++) {
        Formula hi = until->lhs();
        Formula psi = until->rhs();
        for (size_t j = 0; j < S.size(); j++) {
            if (S[j].find(psi) != S[j].end()) {
                S[j].insert(*until);
            }
            else if (S[j].find(hi) != S[j].end()) {
                to_add.push_back(S[j]);
                S[j].insert(*until);
            }
        }
    }

    S.insert(S.end(), to_add.begin(), to_add.end());
}

bool is_true_until(Formula f, std::unordered_set<Formula>& true_set) {
    if (f.kind() == Formula::AND) {
        return is_true_until(f.lhs(), true_set) && is_true_until(f.rhs(), true_set);
    }
    else if (f.kind() == Formula::OR) {
        return is_true_until(f.lhs(), true_set) || is_true_until(f.rhs(), true_set);
    }
    else if (f.kind() == Formula::NOT) {
        return !is_true_until(f.lhs(), true_set);
    }
    else if (f.kind() == Formula::ATOM || f.kind() == Formula::X) {
        return true_set.find(f) != true_set.end();
    }
    else if (f.kind() == Formula::U){
        return true_set.find(f) != true_set.end();
    } else {
        return false;
    }
}

void add_states(Automaton& res, Formula& formula, std::unordered_set<Formula>& untils, std::vector<std::unordered_set<Formula>>& S) {
    for (int i = 0; i < S.size(); i++) {
        bool init = false;
        bool fin = false;
        if (is_true_until(formula, S[i])) {
            init = true;
        }
        for (auto until = untils.begin(); until != untils.end(); until++) {
            Formula psi = until->rhs();
            if ((S[i].find(*until) == S[i].end()) || ((S[i].find(psi) != S[i].end()))) {
                fin = true;
            }
        }
        res.add_state("s" + std::to_string(i));
        if (init) {
            res.set_initial("s" + std::to_string(i));
        }
        if (fin) {
            res.set_final("s" + std::to_string(i), 1);
        }
    }
}

void add_transition(Automaton& res, int num, std::vector<std::unordered_set<Formula>>& S, std::unordered_set<Formula>& untils, std::unordered_set<Formula>& closure) {
    std::unordered_set<Formula> X_should;
    std::unordered_set<Formula> X_shouldnt;
    std::unordered_set<Formula> U_should;
    std::unordered_set<Formula> U_shouldnt;

    std::unordered_set<Formula> Xes = get_X(closure);
    std::set<std::string> atoms = {};

    for (auto it = S[num].begin(); it != S[num].end(); it++) {
        if (it->kind() == Formula::ATOM) {
            atoms.insert(it->prop());
        }
    }

    for (auto next = Xes.begin(); next != Xes.end(); next++) {
        if (S[num].find(*next) != S[num].end()) {
            X_should.insert(next->lhs());
        }
        else {
            X_shouldnt.insert(next->lhs());
        }
    }

    for (auto until = untils.begin(); until != untils.end(); until++) {
        Formula psi = until->rhs();
        Formula hi = until->lhs();
        if (S[num].find(*until) != S[num].end()) {
            if (S[num].find(psi) == S[num].end()) {
                U_should.insert(*until);
            }
        }
        else {
            if (S[num].find(hi) != S[num].end()) {
                U_shouldnt.insert(*until);
            }
        }
    }

    for (size_t i = 0; i < S.size(); i++) {
        bool is_transition = true;
        for (auto it = X_should.begin(); it != X_should.end(); it++) {
            if (S[i].find(*it) == S[i].end()) {
                is_transition = false;
                break;
            }
        }
        if (is_transition)
            for (auto it = X_shouldnt.begin(); it != X_shouldnt.end(); it++) {
                if (S[i].find(*it) != S[i].end()) {
                    is_transition = false;
                    break;
                }
            }
        if (is_transition)
            for (auto it = U_should.begin(); it != U_should.end(); it++) {
                if (S[i].find(*it) == S[i].end()) {
                    is_transition = false;
                    break;
                }
            }
        if (is_transition)
            for (auto it = U_shouldnt.begin(); it != U_shouldnt.end(); it++) {
                if (S[i].find(*it) != S[i].end()) {
                    is_transition = false;
                    break;
                }
            }
        if (is_transition) {
            res.add_trans("s" + std::to_string(num), atoms, "s" + std::to_string(i));
        }
    }
}

void add_transitions(Automaton& res, std::vector<std::unordered_set<Formula>>& S, std::unordered_set<Formula>& untils, std::unordered_set<Formula>& closure) {
    for (int i = 0; i < S.size(); i++) {
        add_transition(res, i, S, untils, closure);
    }
}

void simplify(std::vector<Formula>& stack) {
    Formula top = stack[stack.size() - 1];
    if (top.kind() == Formula::ATOM) {
        return;
    }
    else if (top.kind() == Formula::X) {
        if (top.lhs().kind() == Formula::OR) {
            stack.push_back(X(top.lhs().lhs()));
            simplify(stack);
            size_t tmp = stack.size() - 1;
            stack.push_back(X(top.lhs().rhs()));
            simplify(stack);
            stack.push_back(stack[tmp] || stack[stack.size() - 1]);
            stack.push_back(X(stack[stack.size() - 1]));
        }
        else if (top.lhs().kind() == Formula::NOT) {
            stack.push_back(top.lhs().lhs());
            simplify(stack);
            stack.push_back(X(stack[stack.size() - 1]));
            stack.push_back(!(stack[stack.size() - 1]));
        }
        else {
            stack.push_back(top.lhs());
            simplify(stack);
            stack.push_back(X(stack[stack.size() - 1]));
        }
    }
    else if (top.kind() == Formula::F) {
        stack.push_back(P("T"));
        simplify(stack);
        stack.push_back(top.lhs());
        simplify(stack);
        stack.push_back(U(stack[stack.size() - 2], stack[stack.size() - 1]));
    }
    else if (top.kind() == Formula::G) {
        stack.push_back(top.lhs());
        simplify(stack);
        stack.push_back(P("T"));
        stack.push_back(!(stack[stack.size() - 2]));
        stack.push_back(U(stack[stack.size() - 2], stack[stack.size() - 1]));
        stack.push_back(!(stack[stack.size() - 1]));
    }
    else if (top.kind() == Formula::NOT) {
        stack.push_back(top.lhs());
        simplify(stack);
        stack.push_back(!(stack[stack.size() - 1]));
    }
    else if (top.kind() == Formula::AND) {
        stack.push_back(top.lhs());
        simplify(stack);
        Formula tmp = stack[stack.size() - 1];
        stack.push_back(top.rhs());
        simplify(stack);
        stack.push_back(tmp && stack[stack.size() - 1]);
    }
    else if (top.kind() == Formula::OR) {
        stack.push_back(top.lhs());
        simplify(stack);
        Formula tmp = stack[stack.size() - 1];
        stack.push_back(top.rhs());
        simplify(stack);
        stack.push_back(tmp || stack[stack.size() - 1]);
    }
    else if (top.kind() == Formula::IMPL) {
        stack.push_back(top.lhs());
        simplify(stack);
        size_t tmp = stack.size() - 1;
        stack.push_back(top.rhs());
        simplify(stack);
        stack.push_back(!(stack[tmp]));
        stack.push_back(stack[stack.size() - 1] || stack[stack.size() - 2]);
    }
    else if (top.kind() == Formula::R) {
        // stack.push_back(top.lhs());
        // simplify(stack);
        // int lhs = stack.size();
        // stack.push_back(G(stack[stack.size() - 1]));
        // simplify(stack);
        // int g = stack.size() - 1;
        // stack.push_back(top.rhs());
        // simplify(stack);
        stack.push_back(top.lhs());
        int lhs = stack.size() - 1;
        stack.push_back(top.rhs());
        int rhs = stack.size() - 1;
        stack.push_back(G(stack[lhs]));
        simplify(stack);
        int g = stack.size() - 1;
        stack.push_back(stack[lhs] && stack[rhs]);
        stack.push_back(U(stack[lhs], stack[stack.size() - 1]));
        simplify(stack);
        stack.push_back(stack[stack.size() - 1] || stack[g]);
    }
}

Automaton formula_to_automaton(Formula& formula, std::vector<Formula> &save) {
    std::cout << "Formula: " << std::endl;
    std::cout << formula << std::endl;
    simplify(save);
    auto simple_formula = save[save.size() - 1];
    std::cout << "Simplified formula: " << std::endl << simple_formula << std::endl;
    Automaton res;
    bool has_true = false;
    std::unordered_set<Formula> closure;
    build_enclosure(simple_formula, closure);

    std::cout << "Closure: " << std::endl;
    std::cout << closure << std::endl << std::endl;

    std::unordered_set<Formula> elementary_sets;
    get_elementary_sets(closure, has_true, elementary_sets);
    std::cout << "Elementary sets: " << std::endl;
    std::cout << elementary_sets << std::endl << std::endl;

    std::vector<std::unordered_set<Formula>> S = recombinate_elementary_sets(elementary_sets, closure, has_true);
    std::cout << "Recombination: " << std::endl;
    std::cout << S << std::endl;

    std::unordered_set<Formula> untils = get_until(closure);
    std::cout << "Untils:" << std::endl << untils << std::endl;
    add_until(untils, S);

    std::cout << "With until:" << std::endl << S << std::endl;
    add_states(res, simple_formula, untils, S);
    add_transitions(res, S, untils, closure);

    std::cout << "Automaton: " << std::endl;
    std::cout << res << std::endl;

    return res;
}

int main() {
    std::vector<Formula> save_formula;
    save_formula.reserve(400);
    save_formula.push_back(G(P("p") >> X(P("q"))));
    Automaton automaton = formula_to_automaton(save_formula[save_formula.size() - 1], save_formula);
    return 0;
}