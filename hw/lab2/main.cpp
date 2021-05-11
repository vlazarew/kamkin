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

#include <cassert>
#include <iostream>

#include "bdd.h"
#include "formula.h"

using namespace model::bdd;
using namespace model::logic;

int main() {
    const Formula &formula1 =  x(0) >>  x(1);
    const Formula &formula2 = !x(1) >> !x(0);
    const Formula &formula3 = formula1 == formula2;
    const Formula &formula4 = T;

    std::cout << "Formulae: " << std::endl;
    std::cout << formula1 << std::endl;
    std::cout << formula2 << std::endl;
    std::cout << formula3 << std::endl;
    std::cout << formula4 << std::endl;

    Bdd bdd;

    const Node& root1 = bdd.create(formula1);
    const Node& root2 = bdd.create(formula2);
    assert(&root1 == &root2);

    const Node& root3 = bdd.create(formula3);
    const Node& root4 = bdd.create(formula4);
    assert(&root3 == &root4);

    return 0;
}
