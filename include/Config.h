#ifndef __CONFIG_H__
#define __CONFIG_H__

struct GameConfig {
    int rows = 30;
    int columns = 40;
    int cellSize = 30;
    int menuWidth = 120;
    int maxBridges = 3;
    int maxLines = 7;
    int passengerCapacity = 10;
    float passengerSpeed = 150.0f;
    float trainSpeed = 100.0f;
    
    bool autoSpawn = false; 
    float spawnInterval = 2.0f; // cada cuantos segundos aparece un pasajero por estacion
};

#endif