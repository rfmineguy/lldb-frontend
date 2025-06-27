#include "FileHeirarchy.hpp"
#include <iostream>

FileHeirarchy::FileHeirarchy(): mainRoot(new HeirarchyElement("/")) {}
FileHeirarchy::~FileHeirarchy() {
}
