//
// Created by arsenii on 08.10.2022.
//
#include <iostream>
#include <fstream>
#include "users.h"
#include "room.h"

enum Actions_guest { Check_available_rooms = 1, Check_your_reservations = 2, Cancel_your_booking = 3 };
enum Actions_admin { Who_is_staying = 1, New_room = 2, New_type = 3, Change_type = 4 };

user::user(const std::string &email) {
  this->user_email = email;
}

int user::validation(const std::string &email, const std::string &password) {
  int isAdmin; // 1 if admin
  unsigned long long temp = find_password(email, &isAdmin);
  if (temp == 1) {
    std::cout << "no such username\n";
    return 2;
  } else if (temp != hash(password)) {
    std::cout << "wrong password " << std::endl;
    std::cout << temp << " " << hash(password) << std::endl;
    return 2;
  }
  std::cout << "successful\n";
  return isAdmin;
}

void user::start_actions() {
  //int action;
  while (int action = display_possible_actions()) {
    action_handler(action);
  }
}

unsigned long long user::find_password(const std::string &email, int *isAdmin) {
  if (!std::filesystem::exists("./users/" + email + ".json")) {
    return 1;
  }
  std::ifstream file("./users/" + email + ".json");
  /*if(!file.is_open()){
    return 1;
  }*/
  nlohmann::json j = nlohmann::json::parse(file);
  unsigned long long password = j["user_password"].get<unsigned long long>();
  if (j["user_type"] == "guest") {
    *isAdmin = 0;
  } else {
    *isAdmin = 1;
  }
  file.close();
  return password;
}

int user::user_exists(const std::string &email) {
  std::ifstream file("./users/" + email + ".json");
  if (file.is_open()) {
    file.close();
    return 1;
  }
  return 0;
}

unsigned long long user::hash(const std::string &str) {
  std::hash<std::string> h;
  return h(str);
}

guest::guest(
    const std::string &first_name,
    const std::string &second_name,
    const std::string &email,
    const std::string &password) {
  this->user_first_name = first_name;
  this->user_second_name = second_name;
  this->user_email = email;
  this->user_password = hash(password);
  this->reservations = new linked_list<guest_node>();
  std::ifstream file("./users/" + this->user_email + ".json");
  if (file.peek() == std::ifstream::traits_type::eof()) { // check if the file is empty
    return;
  }
  nlohmann::json j = nlohmann::json::parse(file);// todo make a separate function for it
  int number_of_nodes = j["number_of_nodes"].get<int>();
  guest_node *current_node = this->reservations->get_head();
  for (int i = 0; i < number_of_nodes; ++i) {
    current_node->next = new guest_node(j["reservation_" + std::to_string(i)]["beginning"].get<time_t>(),
                                        j["reservation_" + std::to_string(i)]["ending"].get<time_t>(),
                                        j["reservation_" + std::to_string(i)]["room_type"].get<std::string>());
    current_node = current_node->next;
  }
}

guest::~guest() {
  guest_node *head = this->reservations->get_head()->next;
  delete this->reservations;

  nlohmann::json j;
  j["user_email"] = this->user_email;
  j["user_first_name"] = this->user_first_name;
  j["user_second_name"] = this->user_second_name;
  j["user_password"] = this->user_password;
  int i = 0;
  while (head != nullptr) {
    j["reservation_" + std::to_string(i)] = {{"beginning", head->beginning}, {"ending", head->ending},
                                             {"room_type", head->room_type}};
    guest_node *tmp = head;
    head = head->next;
    delete tmp;
    ++i;
  }
  j["number_of_nodes"] = i;
  j["user_type"] = "guest";
  if (!isChanged) {
    return;
  }
  std::ofstream file("./users/" + this->user_email + ".json");
  file << j;
  file.close();
}

guest::guest(const std::string &email) {
  this->user_email = email;
  std::ifstream file("./users/" + this->user_email + ".json");
  if (file.peek() == std::ifstream::traits_type::eof()) { // check if the file is empty
    return;
  }
  nlohmann::json j = nlohmann::json::parse(file);// todo make a separate function for it
  this->user_first_name = j["user_first_name"].get<std::string>();
  this->user_second_name = j["user_second_name"].get<std::string>();
  this->user_password = j["user_password"].get<unsigned long long>();
  int number_of_nodes = j["number_of_nodes"].get<int>();
  this->reservations = new linked_list<guest_node>();
  guest_node *current_node = this->reservations->get_head();
  for (int i = 0; i < number_of_nodes; ++i) {
    current_node->next = new guest_node(j["reservation_" + std::to_string(i)]["beginning"].get<time_t>(),
                                        j["reservation_" + std::to_string(i)]["ending"].get<time_t>(),
                                        j["reservation_" + std::to_string(i)]["room_type"].get<std::string>());
    current_node = current_node->next;
  }
}

int guest::registration(const std::string &email, const std::string &password) {
  if (user::user_exists(email)) {
    std::cout << "There is a user with given email";
    return 1;
  }
  std::cout << "enter your first name: ";
  std::string first_name;
  std::cin >> first_name;
  std::cout << "enter your second name: ";
  std::string second_name;
  std::cin >> second_name;
  auto *current_user = new guest(first_name, second_name, email, password);
  current_user->add_user_to_file();
  current_user->isChanged = true;
  delete (current_user);
  return 0;
}

int guest::add_user_to_file() {
  std::string filename = "./users/" + this->user_email + ".json";
  std::ofstream file(filename);
  nlohmann::json j;
  j["user_email"] = this->user_email;
  j["user_password"] = this->user_password;
  j["user_first_name"] = this->user_first_name;
  j["user_second_name"] = this->user_second_name;
  j["user_type"] = "guest";
  file << j;
  file.close();
  return 0;
}

int guest::display_possible_actions() {
  int choice;
  while (true) {
    std::cout << "Chose what you want to do:\n"
                 "1. Check available rooms\n"
                 "2. View your bookings\n"
                 "3. Cancel your reservations\n"
                 "0. Exit program\n";
    std::cin >> choice;
    if (choice < 4 && choice >= 0) {
      return choice;
    }
    std::cout << "Wrong number\n" << std::endl;
  }
}

tm *get_dates() {
  std::cout << "Year: ";
  tm *dates = new tm();
  std::cin >> dates->tm_year;
  std::cout << "Month: ";
  std::cin >> dates->tm_mon;
  std::cout << "Day: ";
  std::cin >> dates->tm_mday;
  return dates;
}

void guest::action_handler(int action) {
  switch (action) {
    case Check_available_rooms: {//check available rooms
      std::cout << "Enter your desired dates of the beginning and ending of your stay.\n"
                   "Beginning:\n";
      tm *beginning = get_dates();
      std::cout << "Ending:\n";
      tm *ending = get_dates();
      time_t beg = mktime(beginning);
      time_t end = mktime(ending);
      std::cout << "Enter desired price range:\n"
                   "From: ";
      unsigned int lower_edge;
      std::cin >> lower_edge;
      std::cout << "To: ";
      unsigned int higher_edge;
      std::cin >> higher_edge;
      all_room_types *x = new all_room_types();
      x->rooms_within_range(beg, end, lower_edge, higher_edge);
      delete x;
      std::string room_type;
      while (true) {
        std::cout << "Enter chosen room type (0 if you choose nothing): ";
        std::cin >> room_type;
        if (room_type != "0") {
          if (std::filesystem::exists("./rooms/" + room_type + ".json")) {
            room_types *y = new room_types(room_type);
            if (!y->book(beg, end, this->user_email)) {
              delete y;
              guest_node *new_reservation = new guest_node(beg, end, room_type);
              this->reservations->add_node(new_reservation);
              this->isChanged = true;
              break;
            } else {
              delete y;
              std::cout << "Sorry, all of the rooms of this type are booked on this period";
            }
          } else {
            std::cout << "Wrong room type, try again\n";
          }
        } else {
          break;
        }
      }
      break;
    }
    case Check_your_reservations: { //view your bookings
      if (this->reservations->get_head()->next == nullptr) {
        std::cout << "You do not have reservations\n";
        break;
      }
      this->reservations->get_head()->next->print_info_about_reservations();
      break;
    }
    case Cancel_your_booking: { // cancel reservations
      std::cout << "Enter beginning of the reservation:\n";
      tm *beginning = get_dates();
      std::cout << "Ending:\n";
      tm *ending = get_dates();
      time_t beg = mktime(beginning);
      time_t end = mktime(ending);
      std::cout << "Enter booked room type: ";
      std::string room_type;
      std::cin >> room_type;
      if (!this->reservations->delete_node(new guest_node(beg, end, room_type))) {
        this->isChanged = true;
        room_types *tmp = new room_types(room_type);
        tmp->delete_reservation(new room_node(beg, end, this->user_email));
        delete tmp;
      } else {
        std::cout << "Reservation not found\n" << std::endl;
      }
      break;
    }
    default: {
      std::cout << "Impossible!\n";
    }
  }
}

int admin::display_possible_actions() {
  int choice;
  while (true) {
    std::cout << "Chose what you want to do:\n"
                 "1. Check who is staying in the room\n"
                 "2. Add new room to the system \n"
                 "3. Add new room type to the system\n"
                 "4. Change something in existing room type\n"
                 "0. Exit program\n";
    std::cin >> choice;
    if (choice < 5 && choice >= 0) {
      return choice;
    }
    std::cout << "Wrong number, try again\n" << std::endl;
  }
}

void admin::action_handler(int action) {
  switch (action) {
    case Who_is_staying: { // check who is staying in the room
      std::cout << "Enter room number: ";
      unsigned int room_number;
      std::cin >> room_number;
      std::cout << admin::who_is_staying_in_room(room_number);
      break;
    }
    case New_room: { // add new room
      std::cout << "Enter room type: ";
      std::string room_type;
      std::cin >> room_type;
      if (!all_room_types::check_if_room_type_exists(room_type)) {
        std::cout << "There is no room of this type";
        return;
      }
      room_types *rooms = new room_types(room_type);
      std::cout << "Enter number of new rooms: ";
      int number_of_rooms;
      std::cin >> number_of_rooms;
      rooms->increase_number_of_rooms(number_of_rooms);
      delete (rooms);
      break;
    }
    case New_type: { //new room type
      std::cout << "Enter a name for a new room type: ";
      std::string room_type;
      std::cin >> room_type;
      std::cout << "Enter price per night for this room type: ";
      unsigned int price_per_night;
      std::cin >> price_per_night;
      all_room_types *all_rooms = new all_room_types();
      all_rooms->add_new_type(room_type, price_per_night);
      delete (all_rooms);
      break;
    }
    case Change_type: { //change info about room_type
      std::cout << "Enter room type: ";
      std::string room_type;
      std::cin >> room_type;
      if (!all_room_types::check_if_room_type_exists(room_type)) {
        std::cout << "There is no room of this type";
        break;
      }
      std::cout << "What do you want to change?\n"
                   "1. Price\n"
                   "0. Nothing";
      int choice;
      std::cin >> choice;
      switch (choice) {
        case 1: {
          all_room_types *rooms = new all_room_types();
          rooms->change_price(room_type);
          delete (rooms);
          break;
        }
        default:break;
      }
      break;

    }
    default: {
      break;
    }
  }
}

std::string admin::who_is_staying_in_room(unsigned int room_number) {

  std::ifstream file("./rooms/" + std::to_string(room_number) + ".json");
  nlohmann::json j = nlohmann::json::parse(file);
  std::string email = j["occupied_by"].get<std::string>();
  file.close();
  return email;
}

admin::admin(const std::string &email) {
  this->user_email = email;
}

void admin::admin_registration(const std::string &email, const std::string &password) {
  std::string filename = "./users/" + email + ".json";
  std::ofstream file(filename);
  nlohmann::json j;
  j["user_email"] = email;
  j["user_password"] = user::hash(password);
  j["user_type"] = "admin";
  file << j;
  file.close();
}