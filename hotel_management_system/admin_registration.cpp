//
// Created by arsenii on 04.11.2022.
//
#include <iostream>
#include "users.h"
int main()
{
  std::cout << "Registration of admin user.\n"
               "Enter email: ";
  std::string email;
  std::cin >> email;
  std::cout << "Enter password: ";
  std::string password;
  std::cin >> password;
  admin::admin_registration(email, password);
  return 0;
}