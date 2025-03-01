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
const GSV::InformationState output_state = std::visit(
    GSV::Evaluator(),
    expression,
    std::variant<std::pair<InformationState, const IModel*>>(
        std::make_pair(input_state, &model)
    )
);
```

`GSV::Evaluator()` takes an `InformationState` and an `const IModel*` as input parameters, and gives back an (updated) `InformationState` as output. Due to the way `std::visit` is implemented in C++, you need to pass both input parameters wrapped in a `std::variant`. If you want a simpler way of handling the visitor call, you may also access the `GSV::Evaluator()` visitor through the `GSV::evaluate()` convenience function:

```c++
// InformatoinState GSV::evaluate(const Expression& expr,
//                                const InformationState& input_state,
//                                const IModel& m);

const InformationState output_state = GSV::evaluate(expr, input_state, model);
```

### Semantic relations

Besides the GSV evaluator visitor, the GSV Evaluator Library also contains functions implementing semantic concepts like consistency, coherence, entailment and equivalence. These relations are declared in [`semantic_relations.hpp`](GSV/include/semantic_relations.hpp).

### Exceptions

The `GSV::Evaluator()` visitor, the `GSV::evaluate()` function, as well as all the functions implementing semantic concepts throw exceptions under certain conditions, detailed in the corresponding documentation.

The two kinds of exceptions thrown are:

- `std::out_of_range`, in one of two cases: when a variable has no binding quantifier or suitable anaphoric antecedent, and when a non-logical term lacks an interpretation in the base `IModel` object.
- `std::invalid_argument`, when the formula or formulas under evaluation have operators or quantifiers not allowed by the GSV grammar (negation, conjunction, disjunction, conditional, existential and universal quantifiers, possibility and necessity operators).

Make sure to catch those exceptions when calling those functions. Keep in mind the following:

- any `std::invalid_argument` exception signals that an invalid formula (relative to GSV grammar) has been passed to the library---you are responsible for providing correct input to the semantic component;
- a `std::out_of_range` exception with the error message "Variable [...] has no anaphoric antecedent or binding quantifier" signals that the interpretation process is undefined for the given formula;
- other `std::out_of_range` exceptions may signal, for example, that some non-logical term (a singular term or predicate) lacks an interpretation in the base model---you are responsible for providing base interpretations for all the non-logical expressions that appear in the formulas under evaluation.

### `IModel` objects

The GSV Evaluator Library expects any object that complies with the `IModel` interface to handle errors through exceptions. Particularly, it is expected that the `termInterpretation()` and `predicateInterpretation()` functions raise an exception if the term or predicate lacks an interpretation in the model. Do not report these errors through special return values (like a negative integer or an empty set), as the GSV Evaluator Library will not interpret those return values as reporting errors.

### Implementing a parser for QML

If you implement a parser for QML formulas, keep in mind that only the following logical expressions are allowd by the GSV grammar:

- **Unary operators**: negation, possibility, necessity.
- **Binary operators**: conjunction, disjunction, conditional.
- **Quantifiers**: existential, universal.

Any logical expression outside of these (e.g., a biconditional, or another kind of quantifier) will not be handled gracefully by the library, and will raise `std::invalid_argument` exception.

## Contributing

Contributions are more than welcome. If you want to contribute, please do the following:

1. Fork the repository.
2. Create a new branch: `git checkout -b feature-name`.
3. Make your changes.
4. Push your branch: `git push origin feature-name`.
5. Create a pull request.

## License
This project is licensed under the [BSD-3-Clause](LICENSE).