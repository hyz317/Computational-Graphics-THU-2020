#pragma once

#include <image.hpp>
#include <algorithm>
#include <queue>
#include <cstdio>
#include <iostream>
#include <queue>

class Element {
public:
    virtual void draw(Image &img) = 0;
    virtual ~Element() = default;
};

class Line : public Element {

public:
    int xA, yA;
    int xB, yB;
    Vector3f color;
    void draw(Image &img) override {
        // std::cout << "now draw line! " << xA << ' ' << yA << ' ' << xB << ' ' << yB << '\n';
        // AlreadyDone: Implement Bresenham Algorithm
        int x = xA, y = yA, dx = abs(xB - xA), dy = abs(yB - yA);
        float k = fabs((float) dy / dx), e = -0.5f;

        // std::cout << "k = " << k << '\n';
        
        if (dx == 0) {
            for (int i = std::min(yA, yB); i <= std::max(yA, yB); i++) {
                img.SetPixel(x, i, color);
            }
        }
        else if (k < 1) {
            for (int i = 0; i <= dx; i++) {
                if (xB >= xA && yB >= yA) img.SetPixel(x, y, color); // Area 8
                if (xB < xA && yB >= yA) img.SetPixel(2 * xA - x, y, color); // Area 5
                if (xB >= xA && yB < yA) img.SetPixel(x, 2 * yA - y, color); // Area 1
                if (xB < xA && yB < yA) img.SetPixel(2 * xA - x, 2 * yA - y, color); // Area 4
                x++;
                e += k;
                if (e >= 0) {
                    y += 1;
                    e -= 1.0f;
                }
            }
        }
        else {
           for (int i = 0; i <= dy; i++) {
                if (xB >= xA && yB >= yA) img.SetPixel(x, y, color); // Area 7
                if (xB < xA && yB >= yA) img.SetPixel(2 * xA - x, y, color); // Area 6
                if (xB >= xA && yB < yA) img.SetPixel(x, 2 * yA - y, color); // Area 2
                if (xB < xA && yB < yA) img.SetPixel(2 * xA - x, 2 * yA - y, color); // Area 3
                y++;
                e += 1 / k;
                if (e >= 0) {
                    x += 1;
                    e -= 1.0f;
                }
            } 
        }


        printf("Draw a line from (%d, %d) to (%d, %d) using color (%f, %f, %f)\n", xA, yA, xB, yB,
                color.x(), color.y(), color.z());
    }
};

class Circle : public Element {

public:
    int cx, cy;
    int radius;
    Vector3f color;
    void draw(Image &img) override {
        // AlreadyDone: Implement Algorithm to draw a Circle       
        int x = 0, y = radius;
        float d = 1.25 - radius;
        circlePoints(x, y, cx, cy, color, img);

        while (x <= y) {
            if (d < 0)
                d += (2 * x + 3);
            else {
                d += (2 * (x - y) + 5);
                y--;
            }
            x++;
            circlePoints(x, y, cx, cy, color, img);
        }

        printf("Draw a circle with center (%d, %d) and radius %d using color (%f, %f, %f)\n", cx, cy, radius,
               color.x(), color.y(), color.z());
    }

    void circlePoints(int x, int y, int cx, int cy, Vector3f color, Image &img) {
        img.SetPixel(cx + x, cy + y, color);
        img.SetPixel(cx + x, cy - y, color);
        img.SetPixel(cx - x, cy + y, color);
        img.SetPixel(cx - x, cy - y, color);
        img.SetPixel(cx + y, cy + x, color);
        img.SetPixel(cx + y, cy - x, color);
        img.SetPixel(cx - y, cy + x, color);
        img.SetPixel(cx - y, cy - x, color);
    }
};

struct Point {
    int x, y;
    Point(int xx, int yy) : x(xx), y(yy) {}
};

class Fill : public Element {

public:
    int cx, cy;
    Vector3f color;
    void draw(Image &img) override {
        // AlreadyDone: Flood fill
        int w = img.Width(), h = img.Height();
        std::queue<Point> q;
        int visited[w][h] = { 0 };
        Vector3f oriColor = img.GetPixel(cx, cy);
        q.push(Point(cx, cy));
        visited[cx][cy] = 1;

        while (!q.empty()) {
            Point p = q.front();
            img.SetPixel(p.x, p.y, color);
            if (p.x != w - 1 && !visited[p.x + 1][p.y] && img.GetPixel(p.x + 1, p.y) == oriColor) { q.push(Point(p.x + 1, p.y)); visited[p.x + 1][p.y] = 1; }
            if (p.y != h - 1 && !visited[p.x][p.y + 1] && img.GetPixel(p.x, p.y + 1) == oriColor) { q.push(Point(p.x, p.y + 1)); visited[p.x][p.y + 1] = 1; }
            if (p.x != 0 && !visited[p.x - 1][p.y] && img.GetPixel(p.x - 1, p.y) == oriColor) { q.push(Point(p.x - 1, p.y)); visited[p.x - 1][p.y] = 1; }
            if (p.y != 0 && !visited[p.x][p.y - 1] && img.GetPixel(p.x, p.y - 1) == oriColor) { q.push(Point(p.x, p.y - 1)); visited[p.x][p.y - 1] = 1; }
            q.pop();
        }

        printf("Flood fill source point = (%d, %d) using color (%f, %f, %f)\n", cx, cy,
                color.x(), color.y(), color.z());
    }
};