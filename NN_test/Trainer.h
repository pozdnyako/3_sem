#ifndef TRAINER_H_INCLUDED
#define TRAINER_H_INCLUDED

#include "../../NeuralNetwork/NeuralNetwork.hpp"
#include "../../StopwatchWin32/Stopwatch/Stopwatch.h"

#include <SFML/Graphics.hpp>

#include <queue>

#define R_CHANAL 0
#define G_CHANAL 1
#define B_CHANAL 2
#define A_CHANAL 3

class Task {
public:
    Task() {}
    Task(int x_0, int y_0): x_0(x_0), y_0(y_0) {}
    Task(int x_0, int y_0,
         std::vector<double> in,
         std::vector<double> out)
         : x_0(x_0), y_0(y_0)
         , in(in),out(out){}

    int x_0, y_0;
    std::vector<double> in;
    std::vector<double> out;
};

#define RANDOM_TASK 0
#define NO_RANDOM_TASK 1

class Trainer {
public:
    static const int AREA_SIZE = 22;
    static const int FRAM_THIC = 4;

    Trainer() :n_tasks(0), isImageLoaded(false), file(NULL) {}
    ~Trainer() { delete network; closeFile(); }

    // Getter
    sf::Vector2u getISize() { return iSize; }
    void getLastResult(std::vector<double> *last) { *last = lastResult; }
    int getResultSize() { return network->property.n_output; }
    sf::Uint8* getPixels() { return pixels; }

    //Loading
    void init_network();
    void loadImage(char* path);

    //Queue works
    Task getCurTask() { if(n_tasks == 0) { ERROR_LOG("task queue is empty") } return tasks.front(); }
    void updateTask() { if(n_tasks == 0) { ERROR_LOG("task queue is empty") } n_tasks --; tasks.pop(); }

    //Core methods
    sf::Vector2u createTask(int flag = RANDOM_TASK, int x_0 = -1,
                                                    int y_0 = -1);
    sf::Vector2u Train();

    // Test
    void FuncSpeedTest();

    int n_tasks;
    std::queue<Task> tasks;

    static const int IN_AREA = 1;
    static const int OUT_AREA = 2;

    int getPixelInfo(int, int, int);
    int area_type(int, int);
private:
    bool isImageLoaded;

    sf::Vector2u iSize;
    sf::Image image;

    sf::Uint8 *pixels;
    NeuralNetwork *network;

    FILE *file;

    std::vector<double> lastResult;

    void printError();
    void closeFile();
};

#endif // TRAINER_H_INCLUDED
