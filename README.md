# RivalLang

RivalLang is my attempt to see how hard is it to make a programing language.
Inspired by Jonathan Blow's JAI programing language and rants.

Created a lexer and parser but stopped at typechecking.
You can't run it but it will create a Abstract Syntax Tree.

# Syntax:

## Function Implementation
Syntax: identifier (variable_declaration, variable_declaration) return_type {
    statements
}

### Example:
```
main () {}
```

`main` is the name of the function

![](./RepoPage/Abstract%20Syntax%20Tree/simple_function_implementation.png)

### Example: 
```
main (param_0 u32, param_1 string = "default value") bool {
    foo u32;
}
```

![](./RepoPage/Abstract%20Syntax%20Tree/complex_function_implementation.png)

---

## Variable declaration
Syntax: identifier type;

Example:
```
foo u32;
```
`foo` is the variable name and `u32` is the type name.

![](./RepoPage/Abstract%20Syntax%20Tree/variable_declaration.png)
---

## Variable assignment
Syntax: adress = expression;

Example:
```
foo = a + b * c;
```
`foo` is the variable name and `a + b * c` is the expression.

![](./RepoPage/Abstract%20Syntax%20Tree/simple_variable_assignment.png)

---

Example:
```
bar.foo = 1 + 2 + 3 * 4 * 5 + 6 * 7;
```
The generated AST will take into consideration the operator precedence.

![](./RepoPage/Abstract%20Syntax%20Tree/variable_assignment.png)

---

