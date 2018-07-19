//
// Created by bartlomiejzarnowski on 18.07.18.
//

#ifndef HEURY_DISTURBER_H
#define HEURY_DISTURBER_H

#include "Fan.h"

class Disturber {

  public:
    Disturber(Fan& fan);
    void update(int humidity);

  private:
    Fan& fan;
    int offTime = 0;
};


#endif //HEURY_DISTURBER_H
