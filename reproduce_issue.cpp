#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>

// Copy of the function from writeAvr.cpp
double parse_ascii_float(const char *s) {
    double sign = 1.0;
    if (*s == '-') { sign = -1.0; ++s; }
    unsigned long ipart = 0;
    while (*s >= '0' && *s <= '9') {
        ipart = ipart * 10 + (unsigned long)(*s - '0');
        ++s;
    }
    double val = (double)ipart;
    if (*s == '.') {
        ++s;
        double place = 0.1;
        while (*s >= '0' && *s <= '9') {
            val += (double)(*s - '0') * place;
            place *= 0.1;
            ++s;
        }
    }
    return sign * val;
}

int main() {
    std::vector<std::string> test_cases = {
        "4.32",
        "4.00",
        "1.234",
        "0.5",
        "-4.32",
        "4.32\r\n",
        "4.32 ",
        " 4.32", // Note: The original function doesn't skip leading spaces, caller does
        "4..32",
        "4.32.1",
        "4,32", // Comma instead of dot
        "4"
    };

    std::cout << std::fixed << std::setprecision(10);

    for (const auto& test : test_cases) {
        double result = parse_ascii_float(test.c_str());
        std::cout << "Input: '" << test << "' -> Output: " << result << std::endl;
    }

    // Specific test for the reported issue
    const char* problem_input = "4.32";
    double problem_result = parse_ascii_float(problem_input);
    if (std::abs(problem_result - 4.32) > 0.000001) {
        std::cout << "FAIL: 4.32 parsed as " << problem_result << std::endl;
    } else {
        std::cout << "PASS: 4.32 parsed correctly as " << problem_result << std::endl;
    }

    return 0;
}
