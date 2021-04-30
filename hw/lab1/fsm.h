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

#pragma once

#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace model {
    namespace fsm {

        class State final {
        public:
            State(const std::string &label, bool init = false, int finish = -1) :
                    _label(label), _initial(init), _final(finish) {}

            bool operator==(const State &rhs) const {
                return _label == rhs._label;
            }

            std::string label() const { return _label; }

            bool is_initial() const { return _initial; }

            bool is_final() const { return _final >= 0; }

            int final_set() const { return _final; }

        private:
            const std::string _label;

            // Initial state flag.
            const bool _initial;
            // Acceptance set index (unless negative).
            const int _final;
        };

        std::ostream &operator<<(std::ostream &out, const State &state) {
            return out << state.label();
        }

        class Transition final {
        public:
            Transition(const State &source, const std::set<std::string> &symbol, const State &target) :
                    _source(source), _symbol(symbol), _target(target) {}

            bool operator==(const Transition &rhs) const {
                return _source == rhs._source
                       && _symbol == rhs._symbol
                       && _target == rhs._target;
            }

            const State &source() const { return _source; }

            const State &target() const { return _target; }

            const std::set<std::string> &symbol() const { return _symbol; }

        private:
            const State &_source;
            const State &_target;
            std::set<std::string> _symbol;
        };

        std::ostream &operator<<(std::ostream &out, const Transition &transition) {
            out << transition.source();
            out << " --[";

            bool separator = false;
            for (auto i = transition.symbol().begin(); i != transition.symbol().end(); i++) {
                if (separator) {
                    out << ", ";
                }
                out << *i;
                separator = true;
            }

            out << "]--> ";
            out << transition.target();

            return out;
        }

        class Automaton final {
        public:
            Automaton() :
                    _trans(1024) {}

            friend std::ostream &operator<<(std::ostream &out, const Automaton &automaton);

            void add_state(const std::string &label, bool init = false, int finish = -1);

            void add_trans(
                    const std::string &source,
                    const std::set<std::string> &symbol,
                    const std::string &target
            );

        private:
            std::unordered_map<std::string, State> _states;
            std::unordered_map<std::string, std::vector<Transition>> _trans;
        };

        inline void Automaton::add_state(const std::string &label, bool init, int finish) {
            State state(label, init, finish);
            _states.insert({label, state});
        }

        inline void Automaton::add_trans(
                const std::string &source,
                const std::set<std::string> &symbol,
                const std::string &target) {

            auto s = _states.find(source);
            auto t = _states.find(target);

            Transition trans(s->second, symbol, t->second);
            _trans[source].push_back(trans);
        }

        std::ostream &operator<<(std::ostream &out, const Automaton &automaton) {
            bool separator = false;

            for (auto i = automaton._trans.begin(); i != automaton._trans.end(); i++) {
                for (auto j = i->second.begin(); j != i->second.end(); j++) {
                    if (separator) {
                        out << std::endl;
                    }
                    out << *j;
                    separator = true;
                }
            }

            return out;
        }

    }
} // namespace model::fsm

