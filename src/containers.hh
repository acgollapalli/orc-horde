/*

SDG                                                                                               JJ

                                       Orc Horde

									  Containers
*/

#pragma once

template<typename T>
struct skySlice {
  size_t size;
  T data[];
};
