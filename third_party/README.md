# Third party dependencies

This folder contains mockups of the external components required by the GSV evaluator library.

## third\_party/semantics

The only third party semantic dependency is a `Model` class that provides suitable interpretations for singular terms and predicates.

The minimal requirements on `Model` are provided by [the `IModel` interface](../GSV/include/interfaces/imodel.hpp).

## third\_party/syntax

This folder contains the implementation of an `Expression` object (over which the interpretation is carried out), and a helper function required to identify variables.