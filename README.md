# Relational Algebra to SQL Translator

A lightweight desktop application built with **C++** and the **FLTK** library that translates relational algebra expressions into their corresponding SQL queries.
This tool provides a simple graphical user interface (GUI) to facilitate the conversion process, making it useful for students and professionals working with database concepts.

---

## ✨ Features

* **Graphical User Interface**: Simple and intuitive GUI built with FLTK.
* **Symbol Palette**: Clickable buttons for common relational algebra operators (σ, π, ⨝, etc.).
* **Recursive Descent Parser**: Handles nested expressions intelligently.
* **Optimized SQL Generation**: Produces clean, readable, and efficient SQL (avoids unnecessary subqueries).
* **Cross-Platform**: Configured for MinGW on Windows, but can be compiled on other platforms as well.

---

## 🛠 Technology Stack

* **Language**: C++
* **GUI Library**: FLTK (Fast, Light Toolkit) version 1.4.x
* **Compiler**: MinGW-w64 (GCC for Windows)
* **IDE/Editor**: Visual Studio Code

---

## ⚙️ Setup and Build Instructions

### 1. Prerequisites

* **MinGW Compiler**: Install MinGW (or MinGW-w64) and add its `bin` directory to your system's `PATH`.
* **FLTK Library**:

  * Place the FLTK source at `C:/dev/libs/fltk-1.4.4`
  * Compile the FLTK library by running `make` inside this directory.
* **VS Code**: Install [Visual Studio Code](https://code.visualstudio.com/) with the **C/C++ extension pack**.

---

### 2. Folder Structure

The project expects the following layout:

```
C:
└── dev
    ├── libs
    │   └── fltk-1.4.4
    └── projects
        └── cp
            ├── .vscode
            │   ├── c_cpp_properties.json
            │   └── tasks.json
            ├── main.cpp
            └── README.md
```

---

### 3. Compilation

1. Open the `cp` folder in Visual Studio Code.
2. Open `main.cpp`.
3. Press **Ctrl + Shift + B** to build the project.

This runs the build task defined in `.vscode/tasks.json`, compiling the source code and linking against FLTK.

If successful, an executable `main.exe` will be created in the `cp` directory.

---

### 4. Running the Application

In the VS Code integrated terminal, run:

```bash
./main.exe
```

The application window should now appear 🎉

---

## 🚀 How to Use

1. Launch the application.
2. Type your **relational algebra expression** into the `RA Expression` input field.

   * Use the symbol buttons for operators like `σ`, `π`, and `⨝`.
3. Click **Translate**.
4. The generated SQL query will appear in the output box.

**Example**

* RA Expression:

  ```
  π name,id (σ major="cs" (students))
  ```
* Generated SQL:

  ```sql
  SELECT name, id FROM students WHERE major="cs";
  ```

---

## 👥 Contributors

* **Shiv**
* **Madhur**
* **Yashraj**
* **Athrv**

---

## 📜 License

This project is open source and available under the [MIT License](LICENSE).
