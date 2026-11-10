#ifndef __CONFIG_H__
#define __CONFIG_H__

struct GameConfig {
    int rows = 30;
    int columns = 40;
    int cellSize = 30;
    int menuWidth = 120;
    int maxBridges = 3;
    int maxLines = 7;
    float passengerSpeed = 150.0f;
};

#endif