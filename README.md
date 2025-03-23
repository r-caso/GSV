# GSV Library

A computational implementation of Groenendijk, Stokhof and Veltman's update semantics for Quantified Modal Logic (QML), developed in reference to:

- Groenendijk, J., Stokhof, M. & Veltman, F. (1996), "Coreference and Modality". In S. Lappin (ed.), *Handbook of Contemporary Semantic Theory*, Blackwell, Oxford, pp. 179-214.

The chapter can be accessed [here](https://stokhof.org/wp-content/uploads/2017/06/groenendijk-stokhof-veltman_cmcmsd.pdf).

This library has been developed as part of work as a Researcher at [IIF/SADAF/CONICET](https://iif.conicet.gov.ar/?lan=en) and as member of the [Talk Group](https://talk-group.org/).

## Components

The library is structured into three primary components:

### 1. GSV Core (`gsv-core`)

The foundational layer that implements semantic primitives:

- Referent systems
- Possibility structures
- Information state representation

A mockup model class for Quantified Modal Logic is provided, but you should implement your own, to suit your needs.

This component provides the basic building blocks for representing semantic content.

### 2. GSV Evaluator (`gsv-evaluator`)

A valuation engine for QMLExpressions:

- Evaluates expressions against semantic models
- Implements interpretation functions
- Provides context-sensitive evaluation

The evaluator bridges between formal expressions and their semantic content.

### 3. GSV Relations (`gsv-relations`)

Defines standard semantic relations:

- Entailment
- Equivalence
- Compatibility
- Consistency
- Coherence
- Other semantic relationships

This component enables reasoning about relationships between different semantic expressions.

## Directory Structure

```
GSV/
├── gsv-core/              # Core semantic primitives
│   ├── include/           # Public headers
│   └── src/               # Implementation files
├── gsv-evaluator/         # Evaluation engine
│   ├── include/           # Public headers
│   └── src/               # Implementation files
├── gsv-relations/         # Semantic relations
│   ├── include/           # Public headers
│   └── src/               # Implementation files
└── interfaces/            # Shared interface definitions
third_party/
└── semantics/             # Third-party semantic libraries
```

## Installation

Clone the repository:
```bash
git clone git@github.com:r-caso/GSV.git
```

### Prerequisites

- C++23 compatible compiler
- CMake 3.22 or newer
- [QMLExpression library](https://github.com/r-caso/QMLExpression)

The GSV library requires the [QMLExpression](https://github.com/r-caso/QMLExpression) library. To install it, follow the instructions in the README (notice that it need not be installed as a system library).

### Building

Once you have a working installation of QMLExpression, to build the QMLParser library, navigate to the QMLParser root folder, and do the following:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
If QMLExpression is not installed as a system library, you have to tell CMake. Instead of cmake .., do:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/QMLExpression
```
Or you can add the following key-value pair in the cacheVariables array of your `CMakePresets.json`:
```bash
"CMAKE_PREFIX_PATH" : "/path/to/QMLExpression"
```

### Installation with CMake
To install GSV as a system library:
```bash
cmake --install .
```

This will install:
- Libraries in `${CMAKE_INSTALL_LIBDIR}`
- Headers in `${CMAKE_INSTALL_INCLUDEDIR}/GSV/`
- CMake package configuration in `${CMAKE_INSTALL_LIBDIR}/cmake/GSV/`

To install GSV to a custom loction (non-system library):
```bash
cmake --install . --prefix /path/to/GSV
```

## Usage

### Including in Your Project

```cmake
find_package(GSV REQUIRED)
target_link_libraries(your_target PRIVATE GSV::GSV)
```

If you have installed GSV as a non-system library, you should tell CMake where to find it.

### QML Formula Evaluation

The main callable within the GSV evaluator library is the `Evaluator()` visitor. To invoke it:

1. First, instantiate a `Model` object that adheres to the `IModel` interface declared in [`imodel.hpp`](GSV/interfaces/imodel.hpp):

```cpp
const int number_of_worlds = 10; // pick your value
const int number_of_individuals = 10; // pick your value

const Model model(number_of_worlds, number_of_individuals);
```

A mockup of a fully working `Model` class is declared in the [`model.hpp`](third_party/semantics/model.hpp) file. If you use this class, you should populate the interpretation functions.

2. Instantiate an `InformationState` object for semantic interpretation:

```cpp
namespace GSV = iif_sadaf::talk::GSV;
const GSV::InformationState input_state = GSV::create(model);
```

Note that `InformationState` is an alias for `std::set<GSV::Possibility>`, and you need to call the `GSV::create()` function to populate the information state with possibilities.

3. Generate an [`Expression` object](third_party/syntax/expression.hpp) that represents a QML formula:

```cpp
// As far as the GSV library is concerned, the parser implementation
// is completely up to you. Something along the following lines is a
// minimal example:

namespace QExpr = iif_sadaf::talk::QMLExpression;

const std::string formula = "Ex Walk(x)";
const QExpr::Expression expr = parse(formula);
```

(If you want an working parser for language of QML, you can check the [QMLParser library](https://github.com/r-caso/QMLParser).)

4. Call the evaluator:

```cpp
const std::expected<GSV::InformationState, std::string> evaluation_result = std::visit(
    GSV::Evaluator(),
    expression,
    std::variant<std::pair<GSV::InformationState, const IModel*>>(
        std::make_pair(input_state, &model)
    )
);
```

For a simpler interface, you can use the `GSV::evaluate()` convenience function:

```cpp
const auto evaluation_result = GSV::evaluate(expr, input_state, model);
```

5. Check the evaluation result:

```cpp
if (!evaluation_result.has_value()) {
    // code to handle error
    // use evaluation_result.error() to access the error message
}
else {
    // code to handle success
    // use evaluation_result.value() to access the output information state
}
```

### Semantic Relations

The GSV library also provides functions implementing semantic concepts like consistency, coherence, entailment and equivalence. These relations are declared in [`semantic_relations.hpp`](GSV/gsv-relations/include/semantic_relations.hpp).

In general, the return value of these functions is `std::expected<bool, std::string>`.

Example usage:

```cpp
const std::string formula_1 = <...>;
const std::string formula_2 = <...>;

const auto expr1 = parse(formula_1);
const auto expr2 = parse(formula_2);

const auto entails_result = GSV::entails(expr1, expr2, infoState);

if (entails_result.has_value() && entails_result.value()) {
    // expr1 entails expr2
}
```

### Error Handling

The `GSV::Evaluator()` visitor, the `GSV::evaluate()` function, and all the functions implementing semantic concepts return error messages under certain conditions, detailed in the corresponding documentation.

### Implementing a Parser for QML

To parse formulas (`std::string`) to `Expression` objects, you may use the [QMLParser library](https://github.com/r-caso/QMLParser). If you decide to implement your own parser for QML formulas, keep in mind that only the following logical expressions are allowed by the GSV grammar (besides the identity predicate):

- **Unary operators**: negation (`QMLExpression::Operator::NEGATION`), possibility (`QMLExpression::Operator::EPISTEMIC_POSSIBILITY`), necessity (`QMLExpression::Operator::EPISTEMIC_NECESSITY`).
- **Binary operators**: conjunction (`QMLExpression::Operator::CONJUNCTION`), disjunction (`QMLExpression::Operator::DISJUNCTION`), conditional (`QMLExpression::Operator::CONDITIONAL`).
- **Quantifiers**: existential (`QMLExpression::Quantifier::EXISTENTIAL`), universal (`QMLExpression::Quantifier::UNIVERSAL`).

Any logical expression outside of these (e.g., a biconditional, or another kind of quantifier) will be handled gracefully by the library, resulting in the return of an error message.

### IModel Objects

The GSV Evaluator Library expects any object that complies with the `IModel` interface to handle errors through the use of `std::expected`. Particularly, it is expected that the `termInterpretation()` and `predicateInterpretation()` functions return an error message if the term or predicate lacks an interpretation in the model.

## Contributing

Contributions are more than welcome. If you want to contribute, please do the following:

1. Fork the repository.
2. Create a new branch: `git checkout -b feature-name`.
3. Make your changes.
4. Push your branch: `git push origin feature-name`.
5. Create a pull request.

## License

This project is licensed under the [BSD-3-Clause](LICENSE).
