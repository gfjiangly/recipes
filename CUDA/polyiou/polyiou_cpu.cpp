// first from https://blog.csdn.net/Bcwan_/article/details/52436690
// from https://github.com/CAPTAIN-WHU/DOTA_devkit
// modified by jiang.g.f
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

#define DEBUG
using namespace std;

#define maxn 1024
const double eps = 1E-8;


/*
*	判断数的符号，==0, return 0; >0, return 1; <0, return -1;
*/
int sig(double d) {
	return (d > eps) - (d < -eps);
}

struct Point {
	double x, y; 
	Point() {}
	Point(double x, double y) :x(x), y(y) {}
	bool operator==(const Point &p) const {
		return sig(x - p.x) == 0 && sig(y - p.y) == 0;
	}
};

/*
*   叉积：有符号的面积
*	绝对值表示三角形面积的两倍，即平行四边形面积，这是叉积的物理意
*	https://zhuanlan.zhihu.com/p/34191849
*/
inline double cross(const Point &o, const Point &a, const Point &b) {
	return (a.x - o.x)*(b.y - o.y) - (b.x - o.x)*(a.y - o.y);
}

/*
*	求任意多边形面积，包括非凸多边形
*/
double area(Point* ps, int n) {
	ps[n] = ps[0];
	double res = 0;
	for (int i = 0; i < n; i++) {
		// 参考点是原点，顺序的两个点与原点构成的两个向量做叉积
		res += ps[i].x*ps[i + 1].y - ps[i].y*ps[i + 1].x;
	}
	return res / 2.0;
}

/*
*	使用叉积判断两线段是否相交
*	如果相交，计算交点
*	https://www.cnblogs.com/xpvincent/p/5208994.html
*/
int lineCross(Point a, Point b, Point c, Point d, Point&p) {
	double s1, s2;
	s1 = cross(a, b, c);
	s2 = cross(a, b, d);
	if (sig(s1) == 0 && sig(s2) == 0) return 2;  // 四点共线
	if (sig(s2 - s1) == 0) return 0;  // 两线段平行（相交于中点时s1、s2异号）
	p.x = (c.x*s2 - d.x*s1) / (s2 - s1);  // |s2-s1|面积和
	p.y = (c.y*s2 - d.y*s1) / (s2 - s1);  // 利用三角形面积比与边长比关系
	return 1;
}

/*
*	多边形切割
*	用直线ab切割多边形p，切割后的在向量(a,b)的左侧，并原地保存切割结果
*	如果退化为一个点，也会返回去,此时n为1
*/
void polygon_cut(Point *p, int &n, const Point &a, const Point &b, Point* pp) {
	// static Point pp[maxn];
	int m = 0; p[n] = p[0];
	for (int i = 0; i < n; i++) {
		if (sig(cross(a, b, p[i])) > 0) pp[m++] = p[i];
		if (sig(cross(a, b, p[i])) != sig(cross(a, b, p[i + 1])))
			lineCross(a, b, p[i], p[i + 1], pp[m++]);
	}
	n = 0;
	for (int i = 0; i < m; i++)
		if (!i || !(pp[i] == pp[i - 1]))
			p[n++] = pp[i];
	while (n > 1 && p[n - 1] == p[0]) n--;
}

// 返回三角形oab和三角形ocd的有向交面积, o是原点
double intersectArea(Point a, Point b, Point c, Point d) {
	Point o(0, 0);
	int s1 = sig(cross(o, a, b));
	int s2 = sig(cross(o, c, d));
	if (s1 == 0 || s2 == 0) return 0.0;  // 退化，面积为0
	if (s1 == -1) swap(a, b);
	if (s2 == -1) swap(c, d);
	Point p[10] = { o,a,b };
	int n = 3;
	Point pp[maxn];
	polygon_cut(p, n, o, c, pp);
	polygon_cut(p, n, c, d, pp);
	polygon_cut(p, n, d, o, pp);
	double res = fabs(area(p, n));
	if (s1*s2 == -1) res = -res; 
	return res;
}

// 求两多边形的交面积
double intersectArea(Point*ps1, int n1, Point*ps2, int n2) {
	if (area(ps1, n1) < 0) reverse(ps1, ps1 + n1);  // 保证面积为正
	if (area(ps2, n2) < 0) reverse(ps2, ps2 + n2);
	ps1[n1] = ps1[0];  // 使多边形首尾相连
	ps2[n2] = ps2[0];
	double res = 0;
	for (int i = 0; i < n1; i++) {
		for (int j = 0; j < n2; j++) {
			res += intersectArea(ps1[i], ps1[i + 1], ps2[j], ps2[j + 1]);
		}
	}
	return res;  // assume res is positive!
}

/*
*	计算两个四边形iou
*	顶点必须按顺时针或逆时针方向有序传入
*/
double poly4_iou(const vector<double> &p, const vector<double> &q) {
	Point ps1[5], ps2[5];
	int n1 = 4;
	int n2 = 4;
	for (int i = 0; i < 4; i++) {
		ps1[i].x = p[i * 2];
		ps1[i].y = p[i * 2 + 1];

		ps2[i].x = q[i * 2];
		ps2[i].y = q[i * 2 + 1];
	}
	double inter_area = intersectArea(ps1, n1, ps2, n2);
	double union_area = fabs(area(ps1, n1)) + fabs(area(ps2, n2)) - inter_area;
	double iou = inter_area / union_area;

#ifdef DEBUG
	cout << "inter_area:" << inter_area << endl;
	cout << "union_area:" << union_area << endl;
	cout << "iou:" << iou << endl;
#endif // DEBUG

	return iou;
}

/*
*	计算两个任意多边形iou, 最大支持1024个边
*   顶点必须按顺时针或逆时针方向有序传入
*/
double poly_iou(const vector<double> &p, const vector<double> &q, int mode=1) {
	Point ps1[maxn], ps2[maxn];
	int n1 = p.size()/2;
	int n2 = q.size()/2;
	for (int i = 0; i < n1; i++) {
		ps1[i].x = p[i * 2];
		ps1[i].y = p[i * 2 + 1];
	}
	for (int i = 0; i < n2; i++) {
		ps2[i].x = q[i * 2];
		ps2[i].y = q[i * 2 + 1];
	}
	double inter_area = intersectArea(ps1, n1, ps2, n2);
	double union_area = 1.0;
	if (mode == 0) union_area = fabs(area(ps1, n1));
	if (mode == 1)
		union_area = fabs(area(ps1, n1)) + fabs(area(ps2, n2)) - inter_area;
	double iou = inter_area / union_area;

#ifdef DEBUG
	cout << "inter_area:" << inter_area << endl;
	cout << "union_area:" << union_area << endl;
	cout << "iou:" << iou << endl;
#endif // DEBUG

	return iou;
}


#ifdef DEBUG
int main() {
	vector<double> p = { 0, 0, 1, 0, 1, 1, 0, 1 };
	vector<double> q = { 0.5, 0.5, 1.5, 0.5, 1.5, 1.5, 0.5, 1.5 };
	poly_iou(p, q);
	poly_iou(p, q, 0);
	poly4_iou(p, q);
	return 0;
}
#endif // DEBUG
