#include <iostream>
#include <string>

#include "Trainer.h"

int pixPos(int x, int y, int chanal) {
    return 4 * (x + y * (Trainer::AREA_SIZE * 2 + 1) ) + chanal;
}

void setPixel(sf::Uint8* pixels, int x, int y, int val, int chanal) {
    pixels[pixPos(x, y, chanal)] = val;
}

void fillImage(sf::Uint8* out_pixels, Trainer *trainer, sf::Vector2u pos, std::vector<double> *results) {
    for(int chanal = 0; chanal < 3; chanal ++) {
        for(int y = 0; y < Trainer::AREA_SIZE; y ++) {
        for(int x = 0; x < Trainer::AREA_SIZE; x ++) {
            setPixel(out_pixels, x, y, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
            if(chanal == R_CHANAL) setPixel(out_pixels, x, y + Trainer::AREA_SIZE + 1, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
            if(chanal == G_CHANAL) setPixel(out_pixels, x, y + Trainer::AREA_SIZE * 2 + 2, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
            if(chanal == B_CHANAL) setPixel(out_pixels, x, y + Trainer::AREA_SIZE * 3 + 3, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
        }}

        int counter = 0;
        for(int y = 0; y < Trainer::AREA_SIZE; y ++) {
        for(int x = 0; x < Trainer::AREA_SIZE; x ++) {
            if(trainer->area_type(x, y) == Trainer::IN_AREA) {
                setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
                if(chanal == R_CHANAL) setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y + Trainer::AREA_SIZE + 1, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
                if(chanal == G_CHANAL) setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y + Trainer::AREA_SIZE * 2 + 2, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
                if(chanal == B_CHANAL) setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y + Trainer::AREA_SIZE * 3 + 3, trainer->getPixelInfo(x + pos.x, y + pos.y, chanal), chanal);
            } else {
                setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y, results->at(counter + chanal) * 255, chanal);
                if(chanal == R_CHANAL) setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y + Trainer::AREA_SIZE * 1 + 1, results->at(counter + chanal) * 255, chanal);
                if(chanal == G_CHANAL) setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y + Trainer::AREA_SIZE * 2 + 2, results->at(counter + chanal) * 255, chanal);
                if(chanal == B_CHANAL) setPixel(out_pixels, x + Trainer::AREA_SIZE + 1, y + Trainer::AREA_SIZE * 3 + 3, results->at(counter + chanal) * 255, chanal);
                counter += 3;
            }
        }}
    }
}

int main() {
    const int FREQ = 10;
    const int SCALE = 7;

    Trainer trainer;
    trainer.init_network();

    trainer.loadImage("pic/1.png");

    sf::Vector2u outSize(Trainer::AREA_SIZE * 2 + 1, Trainer::AREA_SIZE * 4 + 3);

    LOG("size: (%d, %d)", outSize.x, outSize.y)

    sf::RenderWindow window(sf::VideoMode(outSize.x * SCALE, outSize.y * SCALE), "tester");

    sf::Image out;
    out.create(outSize.x, outSize.y);

    sf::Texture texture;
    sf::Sprite sprite;

    sprite.setScale(SCALE, SCALE);

    sf::Uint8* pixels = (sf::Uint8*) out.getPixelsPtr();

    for(int y = 0; y < outSize.y; y ++) {
        setPixel(pixels, Trainer::AREA_SIZE, y, 0, R_CHANAL);
        setPixel(pixels, Trainer::AREA_SIZE, y, 0, G_CHANAL);
        setPixel(pixels, Trainer::AREA_SIZE, y, 0, B_CHANAL);
    }

    for(int x = 0; x < outSize.x; x ++) {
        setPixel(pixels, x, Trainer::AREA_SIZE, 0, R_CHANAL);
        setPixel(pixels, x, Trainer::AREA_SIZE, 0, G_CHANAL);
        setPixel(pixels, x, Trainer::AREA_SIZE, 0, B_CHANAL);

        setPixel(pixels, x, Trainer::AREA_SIZE * 2, 0, R_CHANAL);
        setPixel(pixels, x, Trainer::AREA_SIZE * 2, 0, G_CHANAL);
        setPixel(pixels, x, Trainer::AREA_SIZE * 2, 0, B_CHANAL);

        setPixel(pixels, x, Trainer::AREA_SIZE * 3, 0, R_CHANAL);
        setPixel(pixels, x, Trainer::AREA_SIZE * 3, 0, G_CHANAL);
        setPixel(pixels, x, Trainer::AREA_SIZE * 3, 0, B_CHANAL);
    }

    for(int y = 0; y < outSize.y; y ++) {
    for(int x = 0; x < outSize.x; x ++) {
        setPixel(pixels, x, y, 255, A_CHANAL);
    }}

    sf::Vector2u place;
    win32::Stopwatch sw;

    std::vector<double> last_res;

    int counter = 0;
    while (window.isOpen()) {
        counter ++;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        /// UPDATE
        sw.Reset();
        sw.Start();
        place = trainer.Train();
        sw.Stop();

        /*if(counter > 0) {
            window.close();
        }*/

        LOG("spend %f ms\n", sw.ElapsedMilliseconds());

        if(counter < FREQ + 1) {
            continue;
        }
        counter = 0;

        trainer.getLastResult(&last_res);

        fillImage(pixels, &trainer, place, &last_res);

        //LOG_V("%f", r_last);
        //LOG_V("%f", g_last);
        //LOG_V("%f", b_last);
        //LOG("")

        texture.loadFromImage(out);
        sprite.setTexture(texture);

        window.clear();
        window.draw(sprite);
        window.display();
    }
    /*double _in[] = {0.5, 0.3};
    double _out[] = {0.3, 0.5, 0.4};

    std::vector<double> in = std::vector<double>(_in, _in + sizeof(_in) / sizeof(double));
    std::vector<double> out = std::vector<double>(_out, _out + sizeof(_out) / sizeof(double));

    LOG_V("% .5f", in);
    LOG_V("% .5f", out);

    for(int i = 0; i < 10000; i ++) {
        network->train(in, out);
        printf("\t\tERROR_LOG:%f\n", network->ERROR_LOG);
    }
    */

    return 0;
}
