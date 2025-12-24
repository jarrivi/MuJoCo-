#include "mjpc/tasks/simple_car/simple_car.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

#include <absl/random/random.h>
#include <mujoco/mujoco.h>
#include "mjpc/task.h"
#include "mjpc/utilities.h"

namespace mjpc {

std::string SimpleCar::XmlPath() const {
  return GetModelPath("simple_car/task.xml");
}

std::string SimpleCar::Name() const { return "SimpleCar"; }

// 目标函数残差计算
void SimpleCar::ResidualFn::Residual(const mjModel* model, const mjData* data,
                                     double* residual) const {
  // 位置残差
  residual[0] = data->qpos[0] - data->mocap_pos[0];
  residual[1] = data->qpos[1] - data->mocap_pos[1];

  // 控制残差
  residual[2] = data->ctrl[0];
  residual[3] = data->ctrl[1];
}

// 状态转换与主循环逻辑
void SimpleCar::TransitionLocked(mjModel* model, mjData* data) {
  // 获取当前车辆位置和目标位置
  double car_pos[2] = {data->qpos[0], data->qpos[1]};
  double goal_pos[2] = {data->mocap_pos[0], data->mocap_pos[1]};
  
  // 计算车辆到目标的距离向量
  double car_to_goal[2];
  mju_sub(car_to_goal, goal_pos, car_pos, 2);
  
  // 如果车辆距离目标小于0.2米则随机刷新一个新的目标点
  if (mju_norm(car_to_goal, 2) < 0.2) {
    absl::BitGen gen_;
    data->mocap_pos[0] = absl::Uniform<double>(gen_, -2.0, 2.0);
    data->mocap_pos[1] = absl::Uniform<double>(gen_, -2.0, 2.0);
    data->mocap_pos[2] = 0.01;
  }

  // 调用数据获取函数
  getDashboardData(model, data);
}

// 场景渲染修改
void SimpleCar::ModifyScene(const mjModel* model, const mjData* data,
                             mjvScene* scene) const {
  int car_body_id = mj_name2id(model, mjOBJ_BODY, "car");
  if (car_body_id < 0) return;
  
  double* car_velocity = SensorByName(model, data, "car_velocity");
  if (!car_velocity) return;
  
  double speed_ms = mju_norm3(car_velocity);
  double speed_kmh = speed_ms * 3.6;
  
  double* car_pos = data->xpos + 3 * car_body_id;
  
  char label[100];
  std::snprintf(label, sizeof(label), "Speed: %.1f km/h", speed_kmh);

  if (scene->ngeom < scene->maxgeom) {
    mjvGeom* geom = scene->geoms + scene->ngeom;
    geom->type = mjGEOM_LABEL;
    geom->size[0] = geom->size[1] = geom->size[2] = 0.15;
    geom->pos[0] = car_pos[0];
    geom->pos[1] = car_pos[1];
    geom->pos[2] = car_pos[2] + 0.2;
    geom->rgba[0] = 1.0f;
    geom->rgba[1] = 1.0f;
    geom->rgba[2] = 1.0f;
    geom->rgba[3] = 1.0f;
    std::strncpy(geom->label, label, sizeof(geom->label) - 1);
    geom->label[sizeof(geom->label) - 1] = '\0';
    scene->ngeom++;
  }
}

}  // namespace mjpc

// 仪表盘数据获取函数的具体实现
void getDashboardData(const mjModel* m, mjData* d) {
    // 获取速度数据
    double vx = d->qvel[0];
    double vy = d->qvel[1];
    double speed = sqrt(vx * vx + vy * vy);
    double speed_kmh = speed * 3.6;

    // 获取位置数据
    double pos_x = d->qpos[0];
    double pos_y = d->qpos[1];
    double pos_z = d->qpos[2]; // 定义了 pos_z

    // 获取加速度数据
    double ax = d->qacc[0];
    double ay = d->qacc[1];
    double acc_total = sqrt(ax * ax + ay * ay);

    // 打印数据到控制台
    // 修改处：在下面的打印语句中加入了 pos_z，消除了"未使用变量"的报错
    std::cout << "\r" 
              << "位置 X:" << pos_x << " Y:" << pos_y << " Z:" << pos_z << " | "
              << "速度:" << speed_kmh << " km/h | "
              << "加速度:" << acc_total << " m/s^2      " << std::flush;
}