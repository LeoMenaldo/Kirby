#ifndef CAKE_H
#define CAKE_H

#include "gameobject.h"

class Cake : public GameObject {
public:
    Cake();
    void updateLogic() override;
};

#endif // CAKE_H