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

The main callable within the GSV evaluator library is the `Evaluator()` visitor. To invoke it, you need to instantiate a `Model` object that adheres to the `IModel` interface declared in [``imodel.hpp``](GSV/include/interfaces/imodel.hpp). A mockup of a fully working `Model` class is declared in the [`model.hpp`](third_party/semantics/model.hpp) file. If you use this class, you should populate the interpretation functions. The mockup `Model` class can be instantited as follows:

```c++
const int number_of_worlds = 10; // pick your value
const int number_of_individuals = 10; // pick your value

Model model(number_of_worlds, number_of_individuals);
```

Once you have a `Model` object instantiated, instantiate an `InformationState` object that will be the input to the semantic interpretation:

```c++
namespace GSV = iif_sadaf::talk::GSV;
GSV::InformationState state = GSV::create(model);
```
Notice that `InformationState` is just an alias for `std::set<GSV::Possibility>`, and you need to call the `GSV::create()` function to populate the information state with possibilities.

Then, you need to generate an [`Expression` object](third_party/syntax/expression.hpp) that represents a QML formula, e.g. by calling an appropriate parser on a formula:

```c++
// The parser implementation is completely up to you.
// Something along the following lines is a minimal example:

std::string formula = "Ex Walk(x)";
Expression expr = parse(formula);
```

Since the GSV evaluator library runs the semantic interpretation on the abstract syntactic representation provided by the `Expression` objects, no parser is included as part of the library, nor as an external dependency, but there are many options to implement one.

Once you have the `InformationState` that serves as input to the interpretation process, and the `Expression` object that is to be interpreted, you may call the `Evaluator()` as you would do with any other visitor:

```c++
GSV::InformationState output_state = std::visit(
    GSV::Evaluator(),
    expression,
    std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(state, &model))
);
```

`GSV::Evaluator()` takes an `InformationState` and an `const IModel*` as input parameters, and gives back an (updated) `InformationState` as output. Due to the way `std::visit` is implemented in C++, you need to pass both input parameters wrapped in a `std::variant`. If you want a simpler way of handling the visitor call, you can implement the following function:

```c++
GSV::InformationState evaluate(const Expression& e, InformationState s, const IModel& m)
{
    return std::visit(
        GSV::Evaluator(),
        e,
        std::variant<std::pair<InformationState, const IModel*>>(std::make_pair(s, &model))
    );
}
```

## Contributing

Contributions are more than welcome. If you want to contribute, please do the following:

1. Fork the repository.
2. Create a new branch: `git checkout -b feature-name`.
3. Make your changes.
4. Push your branch: `git push origin feature-name`.
5. Create a pull request.

## License
This project is licensed under the [BSD-3-Clause](LICENSE).