#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype> // for isspace and isalnum
#include <ctime>  // for srand
#include <cstdlib> // for rand

// --- Forward Declarations for the Parser ---
// The new signature uses an index 'pos' to track progress
std::string parse_expression(const std::string& expr, size_t& pos);


// --- Helper functions for parsing ---

// Moves the index 'pos' past any leading whitespace
void trim_left(const std::string& s, size_t& pos) {
    while (pos < s.length() && isspace(s[pos])) {
        pos++;
    }
}

// Consumes a specific character from the current position
bool consume(const std::string& s, size_t& pos, char c) {
    trim_left(s, pos);
    if (pos < s.length() && s[pos] == c) {
        pos++;
        return true;
    }
    return false;
}

// Parses a simple identifier (like a table name or attribute) from the current position
std::string parse_identifier(const std::string& expr, size_t& pos) {
    trim_left(expr, pos);
    size_t start_pos = pos;
    while (pos < expr.length() && (isalnum(expr[pos]) || expr[pos] == '_')) {
        pos++;
    }
    if (start_pos == pos) {
        throw std::runtime_error("Expected an identifier (e.g., table name).");
    }
    return expr.substr(start_pos, pos - start_pos);
}


// --- Core Parsing Logic (using index 'pos') ---

// MODIFIED FUNCTION
std::string parse_projection(const std::string& expr, size_t& pos) {
    // 1. Find the attributes list (up to the opening parenthesis)
    trim_left(expr, pos);
    size_t parenthesis_pos = expr.find('(', pos);
    if (parenthesis_pos == std::string::npos) {
        throw std::runtime_error("Syntax error in Project (π): missing '('.");
    }

    std::string attributes = expr.substr(pos, parenthesis_pos - pos);
    attributes.erase(attributes.find_last_not_of(" \t\n\r") + 1);
    
    pos = parenthesis_pos; // Move position to the parenthesis

    // 2. Parse the inner expression
    std::string inner_sql = parse_expression(expr, pos);

    // 3. Build the SQL with smarter FROM clause formatting
    std::string from_clause;
    // Check if inner_sql is a subquery. A simple check is to see if it contains "SELECT".
    if (inner_sql.find("SELECT") != std::string::npos) {
        // It's a subquery, so it needs parentheses and an alias (without AS)
        from_clause = "(" + inner_sql + ") T_" + std::to_string(rand());
    } else {
        // It's a simple table name, no parentheses or alias needed
        from_clause = inner_sql;
    }
    
    return "SELECT " + attributes + " FROM " + from_clause;
}

// MODIFIED FUNCTION
std::string parse_selection(const std::string& expr, size_t& pos) {
    // 1. Find the condition (up to the opening parenthesis)
    trim_left(expr, pos);
    size_t parenthesis_pos = expr.find('(', pos);
    if (parenthesis_pos == std::string::npos) {
        throw std::runtime_error("Syntax error in Select (σ): missing '('.");
    }

    std::string condition = expr.substr(pos, parenthesis_pos - pos);
    condition.erase(condition.find_last_not_of(" \t\n\r") + 1);

    pos = parenthesis_pos; // Move position to the parenthesis

    // 2. Parse the inner expression
    std::string inner_sql = parse_expression(expr, pos);

    // 3. Build the SQL with smarter FROM clause formatting
    std::string from_clause;
    // Check if inner_sql is a subquery.
    if (inner_sql.find("SELECT") != std::string::npos) {
        // It's a subquery, so it needs parentheses and an alias (without AS)
        from_clause = "(" + inner_sql + ") T_" + std::to_string(rand());
    } else {
        // It's a simple table name, no parentheses or alias needed
        from_clause = inner_sql;
    }

    return "SELECT * FROM " + from_clause + " WHERE " + condition;
}


// This is the main recursive function that drives the parser.
std::string parse_expression(const std::string& expr, size_t& pos) {
    trim_left(expr, pos);
    if (pos >= expr.length()) {
        throw std::runtime_error("Unexpected end of expression.");
    }

    // Check for unary operators
    if (expr.substr(pos, 2) == u8"π") {
        pos += 2; // 2 bytes for UTF-8 'π'
        return parse_projection(expr, pos);
    } 
    if (expr.substr(pos, 2) == u8"σ") {
        pos += 2; // 2 bytes for UTF-8 'σ'
        return parse_selection(expr, pos);
    }

    // Check for nested expression in parenthesis
    if (expr[pos] == '(') {
        pos++;
        std::string result = parse_expression(expr, pos);
        if (!consume(expr, pos, ')')) {
            throw std::runtime_error("Syntax error: mismatched parentheses.");
        }
        return result;
    }

    // Base case: must be a table name
    return parse_identifier(expr, pos);
}


// --- Main Entry Point for Translation ---
std::string parse_and_translate(const std::string& input) {
    if (input.empty()) {
        return "Please enter a relational algebra expression.";
    }
    try {
        size_t pos = 0;
        std::string result = parse_expression(input, pos);
        trim_left(input, pos); // Check for trailing characters
        if (pos < input.length()) {
             return "Error: Could not parse entire expression. Remainder starts at: " + input.substr(pos);
        }
        return result + ";";
    } catch (const std::runtime_error& e) {
        return "Parsing Error: " + std::string(e.what());
    }
}


class RelationalAlgebraGUI {
public:
    Fl_Window* window;
    Fl_Input* expression_input;
    Fl_Output* sql_output;

    RelationalAlgebraGUI() {
        // Main Window
        window = new Fl_Window(800, 600, "Relational Algebra to SQL Translator");
        window->begin();

        // Input Box for Relational Algebra expression
        expression_input = new Fl_Input(150, 25, 625, 40, "RA Expression:");
        expression_input->textfont(FL_COURIER);
        expression_input->textsize(16);

        // Symbol Buttons
        create_symbol_buttons();

        // Translate Button
        Fl_Button* translate_btn = new Fl_Button(350, 150, 100, 30, "Translate");
        translate_btn->callback(translate_cb, this);

        // Output Box for SQL Query
        sql_output = new Fl_Output(150, 200, 625, 350, "Generated SQL:");
        sql_output->textfont(FL_COURIER);
        sql_output->textsize(16);
        sql_output->align(FL_ALIGN_TOP_LEFT);

        window->end();
        window->show();
    }

private:
    // --- Callbacks ---
    static void insert_symbol_cb(Fl_Widget* w, void* data) {
        RelationalAlgebraGUI* gui = static_cast<RelationalAlgebraGUI*>(data);
        const char* symbol = w->label();
        gui->expression_input->insert(symbol);
        gui->expression_input->take_focus();
    }

    static void translate_cb(Fl_Widget* w, void* data) {
        RelationalAlgebraGUI* gui = static_cast<RelationalAlgebraGUI*>(data);
        const char* input_text = gui->expression_input->value();
        if (input_text) {
            std::string result = parse_and_translate(input_text);
            gui->sql_output->value(result.c_str());
        }
    }

    void create_symbol_buttons() {
        const char* symbols[] = { u8"σ", u8"π", u8"⨝", u8"∪", u8"∩", u8"-", u8"×" };
        const char* tooltips[] = { "Select (sigma)", "Project (pi)", "Join", "Union", "Intersection", "Difference", "Cartesian Product"};
        int x = 150;
        int y = 80;
        for (int i = 0; i < 7; ++i) {
            Fl_Button* btn = new Fl_Button(x, y, 40, 40, symbols[i]);
            btn->labelsize(20);
            btn->tooltip(tooltips[i]);
            btn->callback(insert_symbol_cb, this);
            x += 50;
        }
    }
};

int main(int argc, char** argv) {
    srand(time(0)); // Seed for random table aliases
    RelationalAlgebraGUI gui;
    return Fl::run();
}

