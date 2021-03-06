#include "Trainer.h"

void Trainer :: init_network() {
    NeuralNetworkProperty *property = new NeuralNetworkProperty();

    property->n_input    = 3 * (AREA_SIZE * AREA_SIZE - (AREA_SIZE - 2 * FRAM_THIC) * (AREA_SIZE - 2 * FRAM_THIC));
    property->n_output   = 3 * (AREA_SIZE - 2 * FRAM_THIC) * (AREA_SIZE - 2 * FRAM_THIC);

    property->n_h_layers = 2;   // HIDDEN LAYERS PROPERTIES
    int n_hidden[] = {500, 500};     // HIDDEN LAYERS PROPERTIES

    property->n_hidden = std::vector<int>(n_hidden,
                                          n_hidden + sizeof(n_hidden) / sizeof(int));

    property->aType_h = Neuron::SIGM;
    property->aType_o = Neuron::RELU;
    property->costFType = 1; // NeuralNetwok.h define

    property->bias = 0.01;
    property->learningRate = 0.005;
    property->momentum = 1;

    property->init();

    network = new NeuralNetwork(*property);
    delete property;
}

int Trainer :: area_type(int x, int y) {
    if(x >= Trainer::FRAM_THIC && x < Trainer::AREA_SIZE - Trainer::FRAM_THIC &&
       y >= Trainer::FRAM_THIC && y < Trainer::AREA_SIZE - Trainer::FRAM_THIC) {

        return OUT_AREA;
    }
    return IN_AREA;
}

int Trainer :: getPixelInfo(int x, int y, int chanal) {
    return pixels[4 * (x + y * iSize.x) + chanal];
}

void Trainer :: loadImage(char* path) {
    image.loadFromFile(path);
    iSize = image.getSize();

    LOG("image size : (%d, %d)", iSize.x, iSize.y);

    pixels = (sf::Uint8*)image.getPixelsPtr();

    /*
    for(int y = 0; y < iSize.y; y ++) {
    for(int x = 0; x < iSize.x; x ++) {
        printf("{%3d, %3d, %3d, %3d} ", getPixelInfo(x, y, R_CHANAL)
                                      , getPixelInfo(x, y, G_CHANAL)
                                      , getPixelInfo(x, y, B_CHANAL)
                                      , getPixelInfo(x, y, A_CHANAL));
    } printf("\n");}

    // PRINT IMAGE by pixels

    */

    isImageLoaded = true;

    int n_x = (iSize.x - AREA_SIZE) + 1;
    int n_y = (iSize.y - AREA_SIZE) + 1;
    int total_ammount = n_x * n_y;
    LOG("total ammount of tasks: %d\n", total_ammount)
}

sf::Vector2u Trainer :: createTask(int flag, int x_0, int y_0) {
    if(!isImageLoaded) {
        ERROR_LOG("image isn't loaded")
    }

    if(flag == RANDOM_TASK) {
        x_0 = rand() % (iSize.x - AREA_SIZE + 1);
        y_0 = rand() % (iSize.y - AREA_SIZE + 1);
    }

    if(x_0 < 0 || x_0 >= iSize.x - AREA_SIZE + 1 ||
       y_0 < 0 || y_0 >= iSize.y - AREA_SIZE + 1) {

        ERROR_LOG("not correct coordinates x_0 and y_0")
    }

    Task task(x_0, y_0);

    #define SET_INFO(type, chanel) \
            task.type.push_back(((double)getPixelInfo(dx + x_0, dy + y_0, chanel)) / 255.0 );

    int n_in = 0, n_out = 0;

    for(int dy = 0; dy < AREA_SIZE; dy ++) {
    for(int dx = 0; dx < AREA_SIZE; dx ++) {
        if(area_type(dx, dy) == IN_AREA) {
            n_in ++;
            SET_INFO(in, R_CHANAL)
            SET_INFO(in, G_CHANAL)
            SET_INFO(in, B_CHANAL)
        } else {
            n_out ++;
            SET_INFO(out, R_CHANAL)
            SET_INFO(out, G_CHANAL)
            SET_INFO(out, B_CHANAL)
        }
    }}

    LOG("created at: (%d, %d)", x_0, y_0);

    #undef SET_INFO

    tasks.push(task);

    n_tasks ++;

    return sf::Vector2u(x_0, y_0);
}

void Trainer :: FuncSpeedTest() {
    win32::Stopwatch sw;

    int n_tasks = 1e7;

    sw.Start();
    for(int i = 0; i < n_tasks; i ++) {
        int x = rand() % iSize.x;
        int y = rand() % iSize.y;

        sf::Color a = image.getPixel(x, y);
    }

    sw.Stop();
    printf("getPixel()\t%d times: %.4f ms\n", n_tasks, sw.ElapsedMilliseconds());


    sw.Reset();
    sw.Start();
    for(int i = 0; i < n_tasks; i ++) {
        int x = rand() % iSize.x;
        int y = rand() % iSize.y;

        sf::Uint8 r = getPixelInfo(x, y, R_CHANAL);
        sf::Uint8 g = getPixelInfo(x, y, G_CHANAL);
        sf::Uint8 b = getPixelInfo(x, y, B_CHANAL);
        sf::Uint8 a = getPixelInfo(x, y, A_CHANAL);
    }

    sw.Stop();
    printf("getPixelInfo\t%d times: %.4f ms\n", n_tasks, sw.ElapsedMilliseconds());

    /// getPixelInfo 4-5 Times Faster!!
}

sf::Vector2u Trainer :: Train() {
    createTask();

    Task cur = getCurTask();

    //pTAB
    //LOG_V("%.2f", cur.in)
    //LOG_V("%.2f", cur.out)
    //mTAB

    network->train(cur.in, cur.out);

    lastResult = network->getActivatedVals(network->property.n_layers - 1);

    updateTask();
    //mTAB

    printError();
    LOG("error: %f", network->error);

    return sf::Vector2u(cur.x_0, cur.y_0);
}

void Trainer :: printError() {
    if(!file) {
        file = fopen("error_stat.txt", "wt");
    }

    fprintf(file, "%f\n", network->error);
}

void Trainer::closeFile() {
    if(file)
        fclose(file);
}
