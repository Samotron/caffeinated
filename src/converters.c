#include "converters.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// Unit conversion
double convert_temperature(double value, const char *from, const char *to) {
    double celsius;
    
    // Convert to Celsius first
    if (strcmp(from, "c") == 0 || strcmp(from, "celsius") == 0) {
        celsius = value;
    } else if (strcmp(from, "f") == 0 || strcmp(from, "fahrenheit") == 0) {
        celsius = (value - 32) * 5.0 / 9.0;
    } else if (strcmp(from, "k") == 0 || strcmp(from, "kelvin") == 0) {
        celsius = value - 273.15;
    } else {
        return -999999;
    }
    
    // Convert from Celsius to target
    if (strcmp(to, "c") == 0 || strcmp(to, "celsius") == 0) {
        return celsius;
    } else if (strcmp(to, "f") == 0 || strcmp(to, "fahrenheit") == 0) {
        return celsius * 9.0 / 5.0 + 32;
    } else if (strcmp(to, "k") == 0 || strcmp(to, "kelvin") == 0) {
        return celsius + 273.15;
    }
    
    return -999999;
}

double convert_length(double value, const char *from, const char *to) {
    double meters;
    
    // Convert to meters
    if (strcmp(from, "m") == 0 || strcmp(from, "meter") == 0) meters = value;
    else if (strcmp(from, "km") == 0 || strcmp(from, "kilometer") == 0) meters = value * 1000;
    else if (strcmp(from, "cm") == 0 || strcmp(from, "centimeter") == 0) meters = value / 100;
    else if (strcmp(from, "mm") == 0 || strcmp(from, "millimeter") == 0) meters = value / 1000;
    else if (strcmp(from, "mi") == 0 || strcmp(from, "mile") == 0) meters = value * 1609.34;
    else if (strcmp(from, "yd") == 0 || strcmp(from, "yard") == 0) meters = value * 0.9144;
    else if (strcmp(from, "ft") == 0 || strcmp(from, "foot") == 0) meters = value * 0.3048;
    else if (strcmp(from, "in") == 0 || strcmp(from, "inch") == 0) meters = value * 0.0254;
    else return -999999;
    
    // Convert from meters
    if (strcmp(to, "m") == 0 || strcmp(to, "meter") == 0) return meters;
    else if (strcmp(to, "km") == 0 || strcmp(to, "kilometer") == 0) return meters / 1000;
    else if (strcmp(to, "cm") == 0 || strcmp(to, "centimeter") == 0) return meters * 100;
    else if (strcmp(to, "mm") == 0 || strcmp(to, "millimeter") == 0) return meters * 1000;
    else if (strcmp(to, "mi") == 0 || strcmp(to, "mile") == 0) return meters / 1609.34;
    else if (strcmp(to, "yd") == 0 || strcmp(to, "yard") == 0) return meters / 0.9144;
    else if (strcmp(to, "ft") == 0 || strcmp(to, "foot") == 0) return meters / 0.3048;
    else if (strcmp(to, "in") == 0 || strcmp(to, "inch") == 0) return meters / 0.0254;
    
    return -999999;
}

double convert_data(double value, const char *from, const char *to) {
    double bytes;
    
    // Convert to bytes
    if (strcmp(from, "b") == 0 || strcmp(from, "byte") == 0) bytes = value;
    else if (strcmp(from, "kb") == 0 || strcmp(from, "kilobyte") == 0) bytes = value * 1024;
    else if (strcmp(from, "mb") == 0 || strcmp(from, "megabyte") == 0) bytes = value * 1024 * 1024;
    else if (strcmp(from, "gb") == 0 || strcmp(from, "gigabyte") == 0) bytes = value * 1024 * 1024 * 1024;
    else if (strcmp(from, "tb") == 0 || strcmp(from, "terabyte") == 0) bytes = value * 1024ULL * 1024 * 1024 * 1024;
    else return -999999;
    
    // Convert from bytes
    if (strcmp(to, "b") == 0 || strcmp(to, "byte") == 0) return bytes;
    else if (strcmp(to, "kb") == 0 || strcmp(to, "kilobyte") == 0) return bytes / 1024;
    else if (strcmp(to, "mb") == 0 || strcmp(to, "megabyte") == 0) return bytes / (1024 * 1024);
    else if (strcmp(to, "gb") == 0 || strcmp(to, "gigabyte") == 0) return bytes / (1024 * 1024 * 1024);
    else if (strcmp(to, "tb") == 0 || strcmp(to, "terabyte") == 0) return bytes / (1024ULL * 1024 * 1024 * 1024);
    
    return -999999;
}

int cmd_convert_unit(const char *value_str, const char *from, const char *to) {
    double value = atof(value_str);
    double result;
    
    // Try temperature
    result = convert_temperature(value, from, to);
    if (result != -999999) {
        printf("%.2f %s = %.2f %s\n", value, from, result, to);
        return 0;
    }
    
    // Try length
    result = convert_length(value, from, to);
    if (result != -999999) {
        printf("%.2f %s = %.2f %s\n", value, from, result, to);
        return 0;
    }
    
    // Try data size
    result = convert_data(value, from, to);
    if (result != -999999) {
        printf("%.2f %s = %.2f %s\n", value, from, result, to);
        return 0;
    }
    
    fprintf(stderr, "Unknown unit conversion: %s to %s\n", from, to);
    fprintf(stderr, "Supported units:\n");
    fprintf(stderr, "  Temperature: c, f, k (celsius, fahrenheit, kelvin)\n");
    fprintf(stderr, "  Length: m, km, cm, mm, mi, yd, ft, in\n");
    fprintf(stderr, "  Data: b, kb, mb, gb, tb\n");
    return 1;
}

// Simple calculator
double evaluate_simple(const char *expr) {
    char cleaned[256];
    int j = 0;
    
    // Remove spaces
    for (int i = 0; expr[i] && j < 255; i++) {
        if (!isspace(expr[i])) {
            cleaned[j++] = expr[i];
        }
    }
    cleaned[j] = '\0';
    
    // Very simple calculator - handles basic operations
    double num1 = 0, num2 = 0;
    char op = 0;
    int i = 0;
    
    // Parse first number
    char num_str[100] = "";
    int idx = 0;
    while (cleaned[i] && (isdigit(cleaned[i]) || cleaned[i] == '.' || cleaned[i] == '-')) {
        num_str[idx++] = cleaned[i++];
    }
    num_str[idx] = '\0';
    num1 = atof(num_str);
    
    // Parse operator
    if (cleaned[i]) {
        op = cleaned[i++];
    }
    
    // Parse second number
    idx = 0;
    while (cleaned[i] && (isdigit(cleaned[i]) || cleaned[i] == '.' || cleaned[i] == '-')) {
        num_str[idx++] = cleaned[i++];
    }
    num_str[idx] = '\0';
    num2 = atof(num_str);
    
    switch (op) {
        case '+': return num1 + num2;
        case '-': return num1 - num2;
        case '*': case 'x': return num1 * num2;
        case '/': return num2 != 0 ? num1 / num2 : 0;
        case '%': return fmod(num1, num2);
        case '^': return pow(num1, num2);
        default: return num1;
    }
}

int cmd_calc(const char *expression) {
    double result = evaluate_simple(expression);
    printf("%.4f\n", result);
    return 0;
}
