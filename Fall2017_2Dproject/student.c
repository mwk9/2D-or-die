#include "student.h"

Student * student(char * name, char * favoriteThing, Sprite * sprite)
{
	Student *person = NULL;
	person = malloc(sizeof(Student));
	person->name = name;
	person->favoriteThing = favoriteThing;
	person->sprite = sprite;
	return person;
}
