//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include <atomic>
#include <typeinfo>
#include "gfx/ManagedObject.hpp"

template<typename T>
using Object = ManagedObject<T>;

template<typename T>
using SharedPtr = ManagedShared<T>;

template<typename T>
using sp = SharedPtr<T>;