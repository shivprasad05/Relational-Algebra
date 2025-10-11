#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <string>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

string parse_expression(const string& expr, size_t& pos);
string parse_term(const string& expr, size_t& pos);

void trim_left(const string& s, size_t& pos) {
    while (pos < s.length() && isspace(s[pos])) {
        pos++;
    }
}

bool consume(const string& s, size_t& pos, char c) {
    trim_left(s, pos);
    if (pos < s.length() && s[pos] == c) {
        pos++;
        return true;
    }
    return false;
}

string parse_identifier(const string& expr, size_t& pos) {
    trim_left(expr, pos);
    size_t start_pos = pos;
    while (pos < expr.length() && (isalnum(expr[pos]) || expr[pos] == '_')) {
        pos++;
    }
    if (start_pos == pos) {
        throw runtime_error("Expected an identifier (e.g., table name).");
    }
    return expr.substr(start_pos, pos - start_pos);
}

string wrap_if_subquery(const string& sql) {
    if (sql.find(' ') == string::npos && sql.find('(') == string::npos) {
        return sql;
    }
    stringstream ss;
    ss << "(" << sql << ") T_" << rand();
    return ss.str();
}

string parse_projection(const string& expr, size_t& pos) {
    trim_left(expr, pos);
    size_t parenthesis_pos = expr.find('(', pos);
    if (parenthesis_pos == string::npos) {
        throw runtime_error("Syntax error in Project (π): missing '('.");
    }
    string attributes = expr.substr(pos, parenthesis_pos - pos);
    attributes.erase(attributes.find_last_not_of(" \t\n\r") + 1);
    pos = parenthesis_pos;
    string inner_sql = parse_expression(expr, pos);
    string from_clause;
    if (inner_sql.find("SELECT") != string::npos || inner_sql.find("UNION") != string::npos) {
        from_clause = wrap_if_subquery(inner_sql);
    } else {
        from_clause = inner_sql;
    }
    return "SELECT " + attributes + " FROM " + from_clause;
}

string parse_selection(const string& expr, size_t& pos) {
    trim_left(expr, pos);
    size_t parenthesis_pos = expr.find('(', pos);
    if (parenthesis_pos == string::npos) {
        throw runtime_error("Syntax error in Select (σ): missing '('.");
    }
    string condition = expr.substr(pos, parenthesis_pos - pos);
    condition.erase(condition.find_last_not_of(" \t\n\r") + 1);
    pos = parenthesis_pos;
    string inner_sql = parse_expression(expr, pos);
    string from_clause;
     if (inner_sql.find("SELECT") != string::npos || inner_sql.find("UNION") != string::npos) {
        from_clause = wrap_if_subquery(inner_sql);
    } else {
        from_clause = inner_sql;
    }
    return "SELECT * FROM " + from_clause + " WHERE " + condition;
}

string parse_term(const string& expr, size_t& pos) {
    trim_left(expr, pos);
    if (pos >= expr.length()) {
        throw runtime_error("Unexpected end of expression.");
    }

    if (expr.compare(pos, 2, u8"π") == 0) {
        pos += 2;
        return parse_projection(expr, pos);
    }
    if (expr.compare(pos, 2, u8"σ") == 0) {
        pos += 2;
        return parse_selection(expr, pos);
    }

    if (expr[pos] == '(') {
        pos++;
        string result = parse_expression(expr, pos);
        if (!consume(expr, pos, ')')) {
            throw runtime_error("Syntax error: mismatched parentheses.");
        }
        return result;
    }

    return parse_identifier(expr, pos);
}

string parse_expression(const string& expr, size_t& pos) {
    string left_sql = parse_term(expr, pos);

    while (true) {
        trim_left(expr, pos);
        
        if (pos < expr.length() && expr.compare(pos, 3, u8"∪") == 0) {
            pos += 3;
            string right_sql = parse_term(expr, pos);
            left_sql = "(" + left_sql + ") UNION (" + right_sql + ")";
        }
        else if (pos < expr.length() && expr.compare(pos, 1, "-") == 0) {
            pos += 1;
            string right_sql = parse_term(expr, pos);
            left_sql = "(" + left_sql + ") EXCEPT (" + right_sql + ")";
        }
        else if (pos < expr.length() && expr.compare(pos, 3, u8"⨝") == 0) {
            pos += 3;
            string right_sql = parse_term(expr, pos);
            left_sql = "SELECT * FROM " + wrap_if_subquery(left_sql) + " NATURAL JOIN " + wrap_if_subquery(right_sql);
        }
        else if (pos < expr.length() && expr.compare(pos, 2, u8"×") == 0) {
            pos += 2;
            string right_sql = parse_term(expr, pos);
            left_sql = "SELECT * FROM " + wrap_if_subquery(left_sql) + " CROSS JOIN " + wrap_if_subquery(right_sql);
        }
        else {
            break;
        }
    }

    return left_sql;
}

string parse_and_translate(const string& input) {
    if (input.empty()) {
        return "Please enter a relational algebra expression.";
    }
    try {
        size_t pos = 0;
        string result = parse_expression(input, pos);
        trim_left(input, pos);
        if (pos < input.length()) {
             return "Error: Could not parse entire expression. Remainder starts at: " + input.substr(pos);
        }
        return result + ";";

    } catch (const runtime_error& e) {
        return "Parsing Error: " + string(e.what());
    }
}

class RelationalAlgebraGUI {
public:
    Fl_Window* window;
    Fl_Input* expression_input;
    Fl_Output* sql_output;

    RelationalAlgebraGUI() {
        window = new Fl_Window(800, 500, "Relational Algebra to SQL Translator");
        window->begin();

        expression_input = new Fl_Input(150, 25, 625, 40, "RA Expression:");
        expression_input->textfont(FL_COURIER);
        expression_input->textsize(16);

        create_symbol_buttons();

        Fl_Button* translate_btn = new Fl_Button(350, 150, 100, 30, "Translate");
        translate_btn->callback(translate_cb, this);

        sql_output = new Fl_Output(150, 200, 625, 150, "Generated SQL:");
        sql_output->textfont(FL_COURIER);
        sql_output->textsize(16);
        sql_output->align(FL_ALIGN_TOP_LEFT);

        const char* syntax_guide_text =
    "--- Sample Syntax ---\n"
    "Select     : σ condition (Relation)\n"
    "Project    : π attributes (Relation)\n"
    "Join       : Relation1 ⋈ Relation2\n"
    "Union      : (Expr1) ∪ (Expr2)\n"
    "Difference : (Expr1) - (Expr2)\n"
    "Product    : Relation1 × Relation2";

        
        Fl_Box* syntax_guide_box = new Fl_Box(150, 360, 625, 100);
        syntax_guide_box->box(FL_NO_BOX);
        syntax_guide_box->labelsize(14);
        syntax_guide_box->labelfont(FL_COURIER);
        syntax_guide_box->label(syntax_guide_text);
        syntax_guide_box->align(FL_ALIGN_TOP | FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        window->end();
        window->show();
    }

private:
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
            string result = parse_and_translate(input_text);
            gui->sql_output->value(result.c_str());
        }
    }

    void create_symbol_buttons() {
        const char* symbols[] = { u8"σ", u8"π", u8"⨝", u8"∪", u8"-", u8"×" };
        const char* tooltips[] = { "Select (sigma)", "Project (pi)", "Join", "Union", "Difference", "Cartesian Product"};
        int x = 150;
        int y = 80;
        for (int i = 0; i < 6; ++i) {
            Fl_Button* btn = new Fl_Button(x, y, 40, 40, symbols[i]);
            btn->labelsize(20);
            btn->labelfont(FL_TIMES_BOLD); 
            btn->tooltip(tooltips[i]);
            btn->callback(insert_symbol_cb, this);
            x += 50;
        }
    }
};

int main(int argc, char** argv) {
    srand(time(0));
    RelationalAlgebraGUI gui;
    return Fl::run();
}
