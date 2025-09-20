#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Animal {
  char *(*speak)(struct Animal *);
  bool (*is_alive)(struct Animal *);

  void *data;
} Animal;

typedef struct {
  size_t age;
  bool is_hungry;
} cat_data;

typedef struct {
  bool wants_bone;
} dog_data;

char *cat_speak(Animal *this) { return "meow"; }
bool cat_is_alive(Animal *this) {
  cat_data *cd = (cat_data *)this->data;
  return cd->is_hungry;
}

char *dog_speak(Animal *this) { return "bark"; }
bool dog_is_alive(Animal *this) {
  dog_data *dd = (dog_data *)this->data;
  return dd->wants_bone;
}

int main() {
  Animal cat = (Animal){.speak = cat_speak,
                        .is_alive = cat_is_alive,
                        .data = (void *)malloc(sizeof(cat_data))};

  Animal dog = (Animal){.speak = dog_speak,
                        .is_alive = dog_is_alive,
                        .data = (void *)malloc(sizeof(dog_data))};

  Animal animals[] = {cat, dog};

  for (size_t i = 0; i < sizeof(animals); i++) {
    Animal a = animals[i];

    a.speak();
  }
}
