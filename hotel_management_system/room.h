#ifndef HOTEL_ROOM_H
#define HOTEL_ROOM_H
#include <ctime>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <filesystem>

//Nodes of linked list, sorted by value of variable "beginning"
class node {
  friend class room;
  friend class room_types;
 protected:
  time_t beginning;
  time_t ending;
};

template<typename T> //todo: make concept of being room_node or guest_node
class linked_list {
 private:
  T *head; //head plays functional role and doesn't contain any information
  void add_node(T *current_head, T *new_node);
  int delete_node(T *current_head, T *comparing_node);
 public:
  T *get_head() { return head; }
  void add_node(T *new_node) { add_node(get_head(), new_node); }
  int delete_node(T *comparing_node) { return delete_node(head, comparing_node); };
  linked_list();
};

class room_node : public node {
  friend class room_types;
  friend class linked_list<room_node>;
 protected:
  std::string email;
  room_node *next = nullptr;

 public:
  room_node(time_t beginning, time_t ending, const std::string &email);
  friend bool operator==(const room_node &lhs, const room_node &rhs);
};

class guest_node : public node {
  friend class guest;
  friend class linked_list<guest_node>;
 private:
  time_t beginning;
  time_t ending;
  std::string room_type;
  guest_node *next = nullptr;

 public:
  void print_info_about_reservations();
  guest_node(time_t beginning, time_t ending, const std::string &room_type);
  friend bool operator==(const guest_node &lhs, const guest_node &rhs);
};

class room_types {
  bool is_changed = false;
  std::string room_type;
  unsigned int first_room_number; // number with which rooms of this type start.
  unsigned int price_per_night;
  linked_list<room_node> *dates_when_occupied;
  unsigned int number_of_rooms; // number of rooms of this type
  void update_bookings(); // writes current bookings into a file
 public:
  linked_list<room_node> *get_dates() { return dates_when_occupied; }
  int rooms_available(time_t beginning, time_t ending); // returns number of available rooms on desired dates
  int book(time_t beginning, time_t ending, const std::string &email); // returns 0 if successful
  void increase_number_of_rooms(unsigned int number_of_new_rooms);
  void change_price_per_night(unsigned int new_price);
  void change_name(const std::string &new_name); // changes room_type
  explicit room_types(const std::string &room_types); // can only be used if room of type room_types already exists
  void delete_reservation(room_node *reservation);
  room_types(const std::string &room_type, unsigned int first_room_number, unsigned int price_per_night,
             unsigned int number_of_rooms); // creates new room type
  ~room_types();
};

class all_room_types {
  bool isChanged = false;
 public:
  std::map<std::string, unsigned int> room_types_list;
  all_room_types();
  void add_new_type(const std::string &room_type, unsigned int price_per_night);
  void rooms_within_range(time_t beginning, time_t ending,
                          unsigned int lower_price_per_night, unsigned int higher_price_per_night);
  static bool check_if_room_type_exists(std::string room_type);
  void change_price(const std::string &room_type);
  void change_name(const std::string &room_type);
  ~all_room_types();
};

class room {
 private:
  unsigned int room_number;
  std::string currently_occupied_by = "free";// "free" if free, email of guest if occupied
  int add_room_to_file();
 public:
  static void create_room_file(unsigned int room_number);
  room(unsigned int room_number, const std::string &current_guest);
  ~room() = default;
};

#endif //HOTEL_ROOM_H