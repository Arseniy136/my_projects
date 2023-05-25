//
// Created by arsenii on 11.11.2022.
//

#ifndef HOTEL__USERS_H_
#define HOTEL__USERS_H_
#include "room.h"
#include <filesystem>

class user {
 protected:
  std::string user_email;
  unsigned long long user_password;
  user() = default;
  explicit user(const std::string &email);

  static unsigned long long find_password(const std::string &email, int *isAdmin);
  static unsigned long long hash(const std::string &str);
  virtual int display_possible_actions() = 0;
  virtual void action_handler(int action) = 0;
 public:
  static int user_exists(const std::string &email);
  static int validation(const std::string &email,
                        const std::string &password); // returns 1 if user is admin, 2 if user dont exist
  void start_actions();
  virtual ~user() {};
};

class guest : public user {
 private:
  std::string user_first_name;
  std::string user_second_name;
  linked_list<guest_node> *reservations;
  bool isChanged = false;
  //guest() = default;
  guest(
      const std::string &first_name,
      const std::string &second_name,
      const std::string &email,
      const std::string &password);

  int add_user_to_file();
  int display_possible_actions() override;
  void action_handler(int action) override;
 public:
  ~guest() override;
  explicit guest(const std::string &email);
  static int registration(const std::string &email, const std::string &password);
};

class admin : public user {
  int display_possible_actions() override;
  void action_handler(int action) override;
  static std::string who_is_staying_in_room(unsigned int room_number);
 public:
  explicit admin(const std::string &email); // requires valid email
  static void admin_registration(const std::string &email, const std::string &password);
  ~admin() override = default;
};

#endif //HOTEL__USERS_H_