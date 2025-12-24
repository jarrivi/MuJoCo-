#ifndef MJPC_DASHBOARD_RENDER_H
#define MJPC_DASHBOARD_RENDER_H

#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846 [cite: 661]
#endif

class DashboardRenderer {
public:
    // 构造函数提供默认值，解决之前 simulate.cc 中的编译报错 
    DashboardRenderer(int window_width = 800, int window_height = 600) 
        : width_(window_width), height_(window_height) {}

    // 渲染主函数：在窗口中绘制所有仪表盘组件 [cite: 668]
    void Render(float win_w, float win_h, double speed_kmh, double rpm, double fuel = 100.0, double temp = 90.0) {
        width_ = static_cast<int>(win_w);
        height_ = static_cast<int>(win_h);

        // 保存当前 OpenGL 状态并切换到 2D 正交投影 [cite: 670, 675]
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width_, 0, height_, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // 基础渲染设置：禁用深度测试确保 UI 覆盖在 3D 场景之上 [cite: 680]
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 绘制速度表：位于屏幕左下角 [cite: 685, 701]
        DrawGauge(150, 150, 120, speed_kmh, 200.0, "KM/H", 1.0f, 1.0f, 1.0f);
        
        // 绘制转速表：位于速度表上方 [cite: 687, 741]
        DrawGauge(150, 420, 100, rpm, 8000.0, "RPM", 0.2f, 1.0f, 0.2f);

        // 绘制辅助信息栏：油量与温度 [cite: 781, 805, 816]
        DrawStatusBar(win_w - 240, 50, 220, 100, fuel, temp);

        // 恢复 OpenGL 原始状态 [cite: 691, 694]
        glDisable(GL_BLEND);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopAttrib();
    }

private:
    int width_, height_;

    // 通用仪表盘绘制函数：支持速度和转速 [cite: 701, 741]
    void DrawGauge(float cx, float cy, float r, double value, double max_val, const char* unit, float pr, float pg, float pb) {
        // 1. 绘制半透明背景圆盘 [cite: 704, 743]
        glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
        DrawFilledCircle(cx, cy, r, 50);

        // 2. 绘制刻度线 [cite: 707, 746]
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glLineWidth(2.0f);
        for (int i = 0; i <= 10; ++i) {
            float angle = M_PI * 0.75f - (static_cast<float>(i) / 10.0f) * M_PI * 1.5f;
            float r_in = r * 0.85f;
            float r_out = r * 0.95f;
            glBegin(GL_LINES);
            glVertex2f(cx + r_in * cos(angle), cy + r_in * sin(angle));
            glVertex2f(cx + r_out * cos(angle), cy + r_out * sin(angle));
            glEnd();
        }

        // 3. 绘制实时指针 [cite: 724, 767]
        float clamp_val = std::min(static_cast<float>(value), static_cast<float>(max_val));
        float pointer_angle = M_PI * 0.75f - (clamp_val / static_cast<float>(max_val)) * M_PI * 1.5f;
        glColor4f(pr, pg, pb, 1.0f); // 指针颜色根据传入参数变化
        glLineWidth(4.0f);
        glBegin(GL_LINES);
        glVertex2f(cx, cy);
        glVertex2f(cx + r * 0.8f * cos(pointer_angle), cy + r * 0.8f * sin(pointer_angle));
        glEnd();
        
        // 绘制中心装饰圆点 [cite: 739, 779]
        glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
        DrawFilledCircle(cx, cy, 10, 20);
    }

    // 绘制状态信息条：油量和温度 [cite: 781, 805, 816]
    void DrawStatusBar(float x, float y, float w, float h, double fuel, double temp) {
        // 背景框 [cite: 785]
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glVertex2f(x + w, y + h); glVertex2f(x, y + h);
        glEnd();

        // 绘制油量条（绿色）[cite: 808]
        float fuel_w = (static_cast<float>(fuel) / 100.0f) * (w - 20);
        glColor4f(0.0f, 0.8f, 0.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x + 10, y + 60); glVertex2f(x + 10 + fuel_w, y + 60);
        glVertex2f(x + 10 + fuel_w, y + 80); glVertex2f(x + 10, y + 80);
        glEnd();

        // 绘制温度条（由蓝变红）[cite: 818, 820, 823]
        float temp_ratio = (static_cast<float>(temp) - 60.0f) / 60.0f;
        float temp_w = std::max(0.0f, std::min(1.0f, temp_ratio)) * (w - 20);
        if (temp_ratio > 0.8f) glColor4f(1.0f, 0.2f, 0.2f, 0.8f); // 过热警告
        else glColor4f(0.2f, 0.5f, 1.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x + 10, y + 20); glVertex2f(x + 10 + temp_w, y + 20);
        glVertex2f(x + 10 + temp_w, y + 40); glVertex2f(x + 10, y + 40);
        glEnd();
    }

    // 辅助函数：绘制实心圆 [cite: 837]
    void DrawFilledCircle(float cx, float cy, float r, int segments) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; i++) {
            float theta = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(segments);
            glVertex2f(cx + r * cos(theta), cy + r * sin(theta));
        }
        glEnd();
    }
};

#endif