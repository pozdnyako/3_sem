#include <SFML/Graphics.hpp>
#include <cstdio>
#include <cmath>

const double M_PI = 3.1415926535;
const double EPS = 0.01;

double calc_angle(const sf::Vector2f &center, const sf::Vector2f &point) {
    //              -90
    //
    // -180        center         0
    //  -------------*--------------> x

    //  180                       0
    //
    //               90

    sf::Vector2f delta = point - center;

    if(std::abs(delta.x) < EPS) {
        if(std::abs(delta.y) < EPS) {
            return 0.0f;
        } else if(delta.y < 0) {
            return -M_PI / 2;
        } else {
            return M_PI / 2;
        }
    }

    if(std::abs(delta.y) < EPS) {
        if(delta.x < 0) {
            return M_PI;
        } else {
            return 0;
        }
    }

    double angle = atan(delta.y / delta.x);

    if(delta.x < 0) {
        if(delta.y < 0) {
            angle -= M_PI;
        } else {
            angle += M_PI;
        }
    }

    return angle;
}

double calc_dist_sq(const sf::Vector2f &a, const sf::Vector2f &b) {
    return (a.x - b.x) * (a.x - b.x) +
           (a.y - b.y) * (a.y - b.y);
}

double calc_dist(const sf::Vector2f &a, const sf::Vector2f &b) {
    return sqrt(calc_dist_sq(a, b));
}

int main() {
    const int WINDOW_X_SIZE = 500, WINDOW_Y_SIZE = 500;
    const float CIRCLE_RADIUS = 50.f;
    const float LINE_LENGTH = 100.0f;
    const float LINE_THICKNESS = 3.0f;

    sf::RenderWindow window(sf::VideoMode(WINDOW_X_SIZE, WINDOW_Y_SIZE), "tester");

    sf::CircleShape shape(CIRCLE_RADIUS);
    shape.setFillColor(sf::Color(50, 50, 50));
    shape.setPosition(WINDOW_X_SIZE / 2 - CIRCLE_RADIUS,
                      WINDOW_Y_SIZE / 2 - CIRCLE_RADIUS);

    sf::Vector2f center = sf::Vector2f(WINDOW_X_SIZE / 2, WINDOW_Y_SIZE / 2);

    const int STATE_CAMERA_MOVE = 0;
    const int STATE_PREPARING = 1;
    const int STATE_BLOCK = 2;

    int state = STATE_CAMERA_MOVE;

    double attack_angle = 0.0f;
    double attack_dist = 0.0f;

    sf::RectangleShape line(sf::Vector2f(LINE_LENGTH, LINE_THICKNESS));
    sf::CircleShape circle(CIRCLE_RADIUS);

    line.setFillColor(sf::Color::Red);
    circle.setFillColor(sf::Color::Red);
    circle.setPosition(WINDOW_X_SIZE / 2 - CIRCLE_RADIUS,
                       WINDOW_Y_SIZE / 2 - CIRCLE_RADIUS);

    while (window.isOpen()) {
        sf::Vector2f cursor = sf::Vector2f(sf::Mouse::getPosition(window));

        double angle = calc_angle(center, cursor);
        double dist = calc_dist(center, cursor);

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            window.close();
        }

        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            state = STATE_PREPARING;
        } else {
            if(state == STATE_PREPARING) {
                attack_angle = angle;
                attack_dist = dist;
            }

            state = STATE_CAMERA_MOVE;
            sf::Mouse::setPosition(sf::Vector2i(center), window);
        }

        window.clear();
        window.draw(shape);

        if(attack_dist < CIRCLE_RADIUS) {
            window.draw(circle);
        } else {
            line.setRotation(attack_angle / M_PI * 180);
            line.setPosition(center.x /*- LINE_LENGTH * cos(attack_angle)*/,
                             center.y /*- LINE_LENGTH * sin(attack_angle)*/);
            window.draw(line);
        }
        window.display();
    }

    return 0;
}
