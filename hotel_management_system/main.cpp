#include <iostream>
#include <vector>
#include "users.h"
#include "room.h"

enum { Sign_in = 1, Sign_up = 2, Wrong_password = 2 };

int main() {
  std::cout << "Enter what you would like to do:\n"
               "1. Sign in\n"
               "2. Sign up\n";
  int action;
  std::cin >> action;
  std::cout << "Enter your email: " << std::endl;
  std::string email;
  std::cin >> email;
  std::cout << "Enter your password: " << std::endl;
  std::string password;
  std::cin >> password;
  int is_admin = 0;
  if (action == Sign_up) {
    guest::registration(email, password);
  } else if (action == Sign_in) {
    if (!user::user_exists(email)) {
      std::cout << "User does not exist";
      return 0;
    }
    while ((is_admin = user::validation(email, password)) == Wrong_password) {
      std::cout << "Enter your password: " << std::endl;
      std::cin >> password;
    }
  } else {
    std::cout << "Wrong number";
    return 0;
  }
  user *x;
  if (is_admin) {
    x = new admin(email);
  } else {
    x = new guest(email);
  }
  x->start_actions();
  delete (x);
  return 0;
}
