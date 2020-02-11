//
// Created by dingjian on 18-2-3.
// Modified by jiang.g.f
//

#ifndef POLYIOU_H
#define POLYIOU_H

#include <vector>
struct Point;
double intersectArea(Point*ps1, int n1, Point*ps2, int n2);
double poly4_iou(const std::vector<double> &p, const std::vector<double> & q);
double poly_iou(const std::vector<double> &p, const std::vector<double> &q, int mode = 1);


#endif //POLYIOU_H
