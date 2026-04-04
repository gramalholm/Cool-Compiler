<h1 align="center">COOL Compiler</h1>

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=c%2B%2B&logoColor=white"/>
  <img src="https://img.shields.io/badge/Build-CMake-green?style=for-the-badge&logo=cmake&logoColor=white"/>
  <img src="https://img.shields.io/badge/Source-COOL-4B8BBE?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Target-BRIL-red?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Status-In_Progress-yellow?style=for-the-badge"/>
</p>

<p align="center">
  An educational project developed in <b>C++</b> and <b>CMake</b> that implements a compiler for the Classroom Object-Oriented Language (COOL), targeting BRIL as the intermediate representation.
</p>

## Authors
* **Gabriel Ramalho** - [Github](https://github.com/gramalholm)
* **Leonardo Nogueira** - [Github](https://github.com/leonardodfn)

---

## 🚧 Development Phases

- [x] [Lexical Analysis](#1-Lexical-Analisys)
- [ ] Syntax Analysis  
- [ ] Semantic Analysis  
- [ ] Code Generation

---

## 1. Lexical Analysis

In this phase, the compiler reads characters from a `.txt` file and groups them into lexemes, which are then converted into tokens.

**Implementation:** We chose to implement the lexical analysis manually, without relying on external tools.

- **Lexer.h**
  In this header file, we define an `enum class` representing all token types of the COOL language.  
  We also implement a class hierarchy for tokens using inheritance, supporting different token types such as string tokens, numeric tokens, and generic tokens.
  Additionally, we define the `Lexer` class and its methods, which form the core of the lexical analysis process.
  
  - **Enum Class**
    
    ```cpp
     enum class lextoken_type {
      // Keywords
      class_token, if_token, then_token, else_token, fi_token, in_kw_token,
      inherits_token, isvoid_token, let_token, loop_token, case_token, while_token,
      esac_token, new_token, of_token, not_token, pool_token, true_token,
      false_token,
  
      // Identifiers and literals
      identifier_token, int_token, string_token, bool_token,
  
      // Operators
      plus_token, minus_token, star_token, slash_token, dot_token, at_token,
  
      // Comparison and assignment
      assign_token, equal_token, greater_token, less_token, greater_equal_token, less_equal_token,
      not_equal_token,
  
      // Delimiters
      semicolon_token, comma_token, colon_token, tilde_token, lparen_token, rparen_token,
      lbrace_token, rbrace_token,
  
      // Special tokens
      error_token, eof_token,
  
      // Type-related tokens
      typeID_token, objectID_token,
    };
    ```
  - **Token Classes**
    
    ```cpp
    class Lextoken{
      private:
          lextoken_type type;
          
      public:
          Lextoken(lextoken_type t);
          lextoken_type get_type() const;
          virtual ~Lextoken() = default;
    };
    ```
    ```cpp
    class StrToken : public Lextoken {
      private:
          std::string token_str;
  
      public:
          StrToken(lextoken_type t, std::string str);
          const std::string& get_token_str() const;
    };
    ```
    ```cpp
    class NumToken : public Lextoken {
    private:
        int token_num;

    public:
        numToken(lextoken_type t, int num);
        int get_token_num() const;
    };
    ```
- **Lexer.cpp**
  In this source file, we implement the core logic of the lexer. The `Lexer` reads the input stream character by character, groups them into lexemes, and generates the corresponding tokens.  

  It is also responsible for:
  - Skipping whitespace and comments  
  - Recognizing keywords, identifiers, and literals  
  - Handling operators and delimiters  
  - Reporting lexical errors when invalid patterns are found   

  * At this stage, we define an unordered_map and initialize it in the Lexer constructor. The map associates strings (keys) with lextoken_type values, allowing us to efficiently determine whether a token is a reserved keyword or an identifier.
  - **Scan() Function**: This is the core of the implementation. In this function, we read characters, group them into lexemes, and analyze them to determine the token type.  
    - The function returns a `std::unique_ptr<Lextoken>` to prevent memory leaks and to leverage polymorphism, allowing the lexer to return different token types (generic, string, or numeric) through a common interface.
---
## 2. Syntax Analysis
The syntax analysis phase is currently under development and will be implemented in future stages of the project.

## 3. Semantic Analysis
The semantic analysis phase is currently under development and will be implemented in future stages of the project.

## 4. Code Generation
The code generation phase is currently under development and will be implemented in future stages of the project.

---

## How to Build the project
## ⚙️ How to Build the Project

### 1. Clone the repository
```bash
git clone https://github.com/your-username/cool-compiler.git
cd cool-compiler
```
### 2. Configure the project
```bash
cmake --preset default
```
### 3. Build
```bash
cmake --build --preset default
```
