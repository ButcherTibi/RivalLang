# RivalLang

RivalLang is my attempt to see how hard is it to make a programing language.
Inspired by Jonathan Blow's JAI programing language and rants.

Created a lexer and parser but stopped at typechecking(name resolution).

Doesn't have code execution but it will create an Abstract Syntax Tree and give error messages with code selection and descriptive message.

Pretty well documented with no external dependencies.

# Contents

- [RivalLang](#rivallang)
- [Contents](#contents)
- [Syntax:](#syntax)
  - [Function Implementation](#function-implementation)
    - [Simplest declaration:](#simplest-declaration)
    - [With parameters and return type:](#with-parameters-and-return-type)
  - [Variable declaration](#variable-declaration)
    - [Simplest:](#simplest)
  - [Variable assignment](#variable-assignment)
    - [Assignment of expression:](#assignment-of-expression)
    - [Operator precedence:](#operator-precedence)
    - [Varied expression:](#varied-expression)
  - [Function call](#function-call)
    - [Simplest call:](#simplest-call)
    - [With varied arguments:](#with-varied-arguments)
- [TODO](#todo)

# Syntax:

## Function Implementation
Syntax: identifier (variable_declaration, variable_declaration) return_type {
    statements
}

### Simplest declaration:
```
main () {}
```

`main` is the name of the function

![](./RepoPage/Abstract%20Syntax%20Tree/simple_function_implementation.png)

### With parameters and return type: 
```
main (param_0 u32, param_1 string = "default value") bool {
    foo u32;
}
```

![](./RepoPage/Abstract%20Syntax%20Tree/complex_function_implementation.png)

---

## Variable declaration
Syntax: identifier type;

### Simplest:
```
foo u32;
```
`foo` is the variable name and `u32` is the type name.

![](./RepoPage/Abstract%20Syntax%20Tree/variable_declaration.png)

---

## Variable assignment
Syntax: adress = expression;

### Assignment of expression:
```
foo = a + b * c;
```
`foo` is the variable name and `a + b * c` is the expression.

![](./RepoPage/Abstract%20Syntax%20Tree/simple_variable_assignment.png)

### Operator precedence:
```
bar.foo = 1 + 2 + 3 * 4 * 5 + 6 * 7;
```
The generated AST will take into consideration the operator precedence.

![](./RepoPage/Abstract%20Syntax%20Tree/variable_assignment.png)

### Varied expression:
```
bar.foo = 0xC0FEBEEF - 10 000U * ("text" + 1.50) / bar('other');
```

![](./RepoPage/Abstract%20Syntax%20Tree/varied_variable_assignment.png)

---

## Function call
Syntax: address(expression_0, expression_1);

### Simplest call:
```
splice();
```
`splice` is the function name.

![](./RepoPage/Abstract%20Syntax%20Tree/function_call.png)

### With varied arguments:
```
splice(0b1100 1111 0000, a + b(), "some text");
```

![](./RepoPage/Abstract%20Syntax%20Tree/function_call_2.png)


# TODO
- demo the error messages
- create testing project to test stuff
- implement comments