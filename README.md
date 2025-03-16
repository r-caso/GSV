# GSV evaluator library

A computational implementation of Groenendijk, Stokhof and Veltman's update semantics for Quantified Modal Logic, developed in:

- Groenendijk, J., Stokhof, M. & Veltman, F. (1996), "Coreference and Modality". In S. Lappin (ed.), *Handbook of
Contemporary Semantic Theory*, Blackwell, Oxford, pp. 179-214.

The chapter can be accessed [here](https://stokhof.org/wp-content/uploads/2017/06/groenendijk-stokhof-veltman_cmcmsd.pdf).

This library has been developed as part of my work as a Researcher at [IIF/SADAF/CONICET](https://iif.conicet.gov.ar/?lan=en) and as member of the [Talk Group](https://talk-group.org/).

## Installation

Just clone the repository:
```bash
git clone git@github.com:r-caso/GSV.git
```

## Usage

### QML formula evaluation

The main callable within the GSV evaluator library is the `Evaluator()` visitor. To invoke it, you need to instantiate a `Model` object that adheres to the `IModel` interface declared in [``imodel.hpp``](GSV/include/interfaces/imodel.hpp). A mockup of a fully working `Model` class is declared in the [`model.hpp`](third_party/semantics/model.hpp) file. If you use this class, you should populate the interpretation functions. The mockup `Model` class can be instantited as follows:

```c++
const int number_of_worlds = 10; // pick your value
const int number_of_individuals = 10; // pick your value

const Model model(number_of_worlds, number_of_individuals);
```

Once you have a `Model` object instantiated, instantiate an `InformationState` object that will be the input to the semantic interpretation:

```c++
namespace GSV = iif_sadaf::talk::GSV;
const GSV::InformationState input_state = GSV::create(model);
```
Notice that `InformationState` is just an alias for `std::set<GSV::Possibility>`, and you need to call the `GSV::create()` function to populate the information state with possibilities.

Then, you need to generate an [`Expression` object](third_party/syntax/expression.hpp) that represents a QML formula, e.g. by calling an appropriate parser on a formula:

```c++
// The parser implementation is completely up to you.
// Something along the following lines is a minimal example:

const std::string formula = "Ex Walk(x)";
const Expression expr = parse(formula);
```

Since the GSV evaluator library runs the semantic interpretation on the abstract syntactic representation provided by the `Expression` objects, no parser is included as part of the library, nor as an external dependency, but there are many options to implement one.

Once you have the `InformationState` that serves as input to the interpretation process, and the `Expression` object that is to be interpreted, you may call the `Evaluator()` as you would do with any other visitor:

```c++
const std::expected<InformationState, std::string> evaluation_result = std::visit(
    GSV::Evaluator(),
    expression,
    std::variant<std::pair<InformationState, const IModel*>>(
        std::make_pair(input_state, &model)
    )
);
```

`GSV::Evaluator()` takes an `InformationState` and a `const IModel*` as input parameters, and returns an (updated) `InformationState` as expected value, and an error message as an unexpected value. Due to the way `std::visit` is implemented in C++, you need to pass both input parameters wrapped in a `std::variant`. If you want a simpler way of handling the visitor call, you may also access the `GSV::Evaluator()` visitor through the `GSV::evaluate()` convenience function:

```c++
// std::expected<InformationState, std::string> evaluate(
//     const Expression& expr,
//     const InformationState& input_state,
//     const IModel& model)

const auto evaluation_result = GSV::evaluate(expr, input_state, model);
```
You can check whether the evaluation was successful by calling the `has_value()` method of `std::expected`:

```c++
if (!evaluation_result.has_value()) {
    // code to handle error
    // use evaluation_result.error() to access the error message
}
else {
    // code to handle success
    // use evaluation_result.value() to access the output information state
}
```

### Semantic relations

Besides the GSV evaluator visitor, the GSV Evaluator Library also contains functions implementing semantic concepts like consistency, coherence, entailment and equivalence. These relations are declared in [`semantic_relations.hpp`](GSV/include/semantic_relations.hpp).

In general, the return value of these functions is `std::expected<bool, std::string>`.

### Error messages

The `GSV::Evaluator()` visitor, the `GSV::evaluate()` function, as well as all the functions implementing semantic concepts return error messages under certain conditions, detailed in the corresponding documentation.

### `IModel` objects

The GSV Evaluator Library expects any object that complies with the `IModel` interface to handle errors through the use of `std::expected`. Particularly, it is expected that the `termInterpretation()` and `predicateInterpretation()` functions return an error message if the term or predicate lacks an interpretation in the model.

### Implementing a parser for QML

If you implement a parser for QML formulas, keep in mind that only the following logical expressions are allowed by the GSV grammar:

- **Unary operators**: negation, possibility, necessity.
- **Binary operators**: conjunction, disjunction, conditional.
- **Quantifiers**: existential, universal.

Any logical expression outside of these (e.g., a biconditional, or another kind of quantifier) will be handled gracefully by the library, resulting in the return of an error message.

## Contributing

Contributions are more than welcome. If you want to contribute, please do the following:

1. Fork the repository.
2. Create a new branch: `git checkout -b feature-name`.
3. Make your changes.
4. Push your branch: `git push origin feature-name`.
5. Create a pull request.

## License
This project is licensed under the [BSD-3-Clause](LICENSE).