#include "input_password.h"

int main() {
    auto passwd = input_password("Please input your password: ");
    fprintf(stderr, "Password: %s\n", passwd.c_str());
    return 0;
}