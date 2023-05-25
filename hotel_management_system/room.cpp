#include "room.h"
#include <filesystem>

template
class linked_list<guest_node>;
template
class linked_list<room_node>;

template<typename T>
linked_list<T>::linked_list() {
  head = new T(0, 0, "a");
}

template<typename T>
void linked_list<T>::add_node(T *current_head, T *new_node) {
  if (current_head->next == nullptr) {
    current_head->next = new_node;
    return;
  }
  if (current_head->next->beginning < new_node->beginning) {
    linked_list::add_node(current_head->next, new_node);
    return;
  }
  new_node->next = current_head->next;
  current_head->next = new_node;
}

template<typename T>
int linked_list<T>::delete_node(T *current_head, T *comparing_node) {
  if (current_head->next == nullptr) {
    return 1;
  }
  if (*current_head->next == *comparing_node) {
    T *temp = current_head->next->next;
    delete (current_head->next);
    current_head->next = temp;
    delete (comparing_node);
    return 0;
  }

  return delete_node(current_head->next, comparing_node);
}

room_node::room_node(time_t beginning, time_t ending, const std::string &email) : node() {
  this->beginning = beginning;
  this->ending = ending;
  this->email = email;
  this->next = nullptr;
}

bool operator==(const room_node &lhs, const room_node &rhs) {
  return (lhs.beginning == rhs.beginning && lhs.ending == rhs.ending && lhs.email == rhs.email);
}

guest_node::guest_node(time_t beginning, time_t ending, const std::string &room_type) : node() {
  this->beginning = beginning;
  this->ending = ending;
  this->room_type = room_type;
  this->next = nullptr;
}

bool operator==(const guest_node &lhs, const guest_node &rhs) {
  return (lhs.beginning == rhs.beginning && lhs.ending == rhs.ending && lhs.room_type == rhs.room_type);
}

void guest_node::print_info_about_reservations() {
  guest_node *temp = this;
  while (temp != nullptr) {
    tm *beg = localtime(&temp->beginning);
    std::cout << "You have a booking in " + temp->room_type + " beginning on " << beg->tm_mday << "." << beg->tm_mon
              << "." << beg->tm_year << " and ending on ";
    tm *end = localtime(&temp->ending);
    std::cout << end->tm_mday << "." << end->tm_mon << "." << end->tm_year << std::endl;
    temp = temp->next;
  }
}

int room::add_room_to_file() {
  nlohmann::json j;
  j["room_number"] = this->room_number;
  j["currently_occupied_by"] = this->currently_occupied_by;
  std::ofstream file("./rooms/" + std::to_string(this->room_number) + ".json");
  if (!file.is_open()) {
    std::cout << "couldn't open the file\n";
    return 1;
  }
  file << j;
  file.close();
  return 0;
}

int room_types::rooms_available(time_t beginning, time_t ending) {
  unsigned int number_of_days = (ending - beginning) / 86400;
  std::vector<int> number_of_guests(number_of_days);
  room_node *current_node = this->dates_when_occupied->get_head()->next;
  while (current_node != nullptr) {
    if (current_node->beginning > ending) {
      break;
    }
    if (current_node->ending < beginning) {
      continue;
    }
    unsigned int number_of_days_covered =
        (std::min(current_node->ending, ending) - std::max(beginning, current_node->beginning)) / 86400;
    unsigned int first_day = std::max(0l, current_node->beginning - beginning) / 86400;
    unsigned int last_day = first_day + number_of_days_covered;
    for (unsigned int i = first_day; i < last_day; ++i) {
      ++number_of_guests[i];
    }
    current_node = current_node->next;
  }
  unsigned int maximum = 0;
  for (int i = 0; i < number_of_days; ++i) {
    if (number_of_guests[i] > maximum) {
      maximum = number_of_guests[i];
    }
  }
  return number_of_rooms - maximum;
}

int room_types::book(time_t beginning_date, time_t ending_date, const std::string &email) {
  if (!rooms_available(beginning_date, ending_date)) {
    std::cout << "no available rooms for this period\n";
    return 1;
  }
  this->dates_when_occupied->add_node(new room_node(beginning_date, ending_date, email));
  this->update_bookings();
  return 0;
}

void room_types::update_bookings() {
  std::ofstream file("./rooms/" + this->room_type + ".json");
  nlohmann::json j;
  j["price_per_night"] = this->price_per_night;
  j["number_of_rooms"] = this->number_of_rooms;
  j["room_type"] = this->room_type;
  j["first_room_number"] = this->first_room_number;
  room_node *head = this->dates_when_occupied->get_head()->next;
  int i = 0;
  while (head != nullptr) {
    j["booking_" + std::to_string(i)] = {{"beginning", head->beginning}, {"ending", head->ending},
                                         {"email", head->email}};
    head = head->next;
    ++i;
  }
  j["number_of_nodes"] = i;
  file << j;
  file.close();
}
void room_types::increase_number_of_rooms(unsigned int number_of_new_rooms) {
  unsigned int max_room_number = this->first_room_number + this->number_of_rooms + number_of_new_rooms - 1;
  for (unsigned int i = this->first_room_number + this->number_of_rooms - number_of_new_rooms; i <= max_room_number;
       ++i) {
    room::create_room_file(i);
  }
  this->number_of_rooms += number_of_new_rooms;
}

void room_types::change_price_per_night(unsigned int new_price) {
  is_changed = true;
  this->price_per_night = new_price;
}

void room_types::change_name(const std::string &new_name) {
  is_changed = true;
  std::string str = "./rooms/" + this->room_type + ".json";
  char *filename = new char[str.length() + 1];
  strcpy(filename, str.c_str());
  remove(filename);
  this->room_type = new_name;
  delete[] filename;
}

room_types::room_types(const std::string &room_types) {
  std::ifstream file("./rooms/" + room_types + ".json");
  nlohmann::json j = nlohmann::json::parse(file);
  this->room_type = room_types;
  this->number_of_rooms = j["number_of_rooms"].get<unsigned int>();
  this->first_room_number = j["first_room_number"].get<unsigned int>();
  this->price_per_night = j["price_per_night"].get<unsigned int>();
  this->dates_when_occupied = new linked_list<room_node>();
  int number_of_nodes = j["number_of_nodes"].get<int>();
  room_node *current_node = this->dates_when_occupied->get_head();
  for (int i = 0; i < number_of_nodes; ++i) {
    current_node->next = new room_node(j["booking_" + std::to_string(i)]["beginning"].get<time_t>(),
                                       j["booking_" + std::to_string(i)]["ending"].get<time_t>(),
                                       j["booking_" + std::to_string(i)]["email"].get<std::string>());
    current_node = current_node->next;
  }
}

room_types::room_types(const std::string &room_type,
                       unsigned int first_room_number,
                       unsigned int price_per_night,
                       unsigned int number_of_rooms) {
  this->room_type = room_type;
  this->first_room_number = first_room_number;
  this->price_per_night = price_per_night;
  this->number_of_rooms = number_of_rooms;
  is_changed = true;
  unsigned int temp = number_of_rooms + first_room_number;
  for (unsigned int i = first_room_number; i < temp; ++i) {
    room::create_room_file(i);
  }
  this->dates_when_occupied = new linked_list<room_node>;
}

void room_types::delete_reservation(room_node *reservation) {
  this->is_changed = true;
  this->dates_when_occupied->delete_node(reservation);

}

room_types::~room_types() {
  if (is_changed) {
    update_bookings();
  }
  room_node *head = this->dates_when_occupied->get_head();
  while (head != nullptr) {
    room_node *temp = head;
    head = head->next;
    delete temp;
  }
}

all_room_types::all_room_types() {
  std::ifstream file("./rooms/room_types.json");
  if (file.peek() == std::ifstream::traits_type::eof()) { // checks if the file is empty
    return;
  }
  nlohmann::json j = nlohmann::json::parse(file);
  int number_of_types = j["number_of_types"].get<int>();
  for (int i = 0; i < number_of_types; ++i) {
    this->room_types_list.insert({j["room_type_" + std::to_string(i)]["type"].get<std::string>(),
                                  j["room_type_" + std::to_string(i)]["price_per_night"].get<unsigned int>()});
  }
}

void all_room_types::add_new_type(const std::string &room_type, unsigned int price_per_night) {
  this->isChanged = true;
  this->room_types_list.insert({room_type, price_per_night});
  std::cout << "Enter number of first room: ";
  unsigned int first_room_number;
  std::cin >> first_room_number;
  std::cout << "Enter number of rooms: ";
  unsigned int number_of_rooms;
  std::cin >> number_of_rooms;
  room_types *x = new room_types(room_type, first_room_number, price_per_night, number_of_rooms);
  delete x;
}

void all_room_types::rooms_within_range(time_t beginning,
                                        time_t ending,
                                        unsigned int lower_price_per_night,
                                        unsigned int higher_price_per_night) {
  for (const auto &it: this->room_types_list) {
    if (it.second > lower_price_per_night && it.second < higher_price_per_night) {
      room_types *temp = new room_types(it.first);
      if (temp->rooms_available(beginning, ending)) {
        std::cout << "There are rooms of type <" + it.first + "> available\n";
        std::cout << "Its price is " << it.second << " per night\n";
      }
    }
  }
}

bool all_room_types::check_if_room_type_exists(std::string room_type) {
  return std::filesystem::exists("./rooms/" + room_type + ".json");
}

void all_room_types::change_price(const std::string &room_type) {
  std::cout << "Old price is " << this->room_types_list[room_type] << ". Enter new price: ";
  unsigned int new_price;
  std::cin >> new_price;
  isChanged = true;
  this->room_types_list[room_type] = new_price;
  room_types *changing_room_type = new room_types(room_type);
  changing_room_type->change_price_per_night(new_price);
  delete changing_room_type;
  return;
}

void all_room_types::change_name(const std::string &room_type) {
  std::cout << "Enter new name: ";
  std::string new_name;
  std::cin >> new_name;
  isChanged = true;
  auto node = this->room_types_list.extract(room_type);
  node.key() = new_name;
  this->room_types_list.insert(std::move(node));
  room_types *changing_room_type = new room_types(room_type);
  changing_room_type->change_name(new_name);
  delete changing_room_type;
  return;
}

all_room_types::~all_room_types() {
  if (this->isChanged) {
    std::ofstream file("./rooms/room_types.json");
    nlohmann::json j;
    int i = 0;
    for (auto &it: this->room_types_list) {
      j["room_type_" + std::to_string(i)] = {{"type", it.first}, {"price_per_night", it.second}};
      ++i;
    }
    j["number_of_types"] = i;
    file << j;
    file.close();
  }
}

void room::create_room_file(unsigned int room_number) {
  if (std::filesystem::exists("./rooms/" + std::to_string(room_number) + ".json")) {
    return;
  }
  nlohmann::json j;
  j["occupied_by"] = "free";
  std::ofstream file("./rooms/" + std::to_string(room_number) + ".json");
  file << j;
  file.close();
}

room::room(unsigned int room_number, const std::string &current_guest) {
  this->room_number = room_number;
  this->currently_occupied_by = current_guest;
}

