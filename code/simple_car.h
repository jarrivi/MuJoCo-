#ifndef MJPC_TASKS_SIMPLE_CAR_SIMPLE_CAR_H_
#define MJPC_TASKS_SIMPLE_CAR_SIMPLE_CAR_H_

#include <string>
#include <mujoco/mujoco.h>
#include "mjpc/task.h"

// 声明获取仪表盘数据的函数
// 我们将其放在mjpc命名空间外以便全局调用
void getDashboardData(const mjModel* m, mjData* d);

namespace mjpc {

class SimpleCar : public Task {
 public:
  std::string Name() const override;
  std::string XmlPath() const override;

  class ResidualFn : public BaseResidualFn {
   public:
    explicit ResidualFn(const SimpleCar* task) : BaseResidualFn(task) {}
    
    // 这里的Residual函数定义了控制目标
    // 包括位置追踪和控制量最小化
    void Residual(const mjModel* model, const mjData* data,
                  double* residual) const override;
  };

  SimpleCar() : residual_(this) {
    visualize = 1;  // 开启可视化
  }

  // 状态转换函数
  // 我们将在这里调用数据获取函数
  void TransitionLocked(mjModel* model, mjData* data) override;

  // 修改场景函数
  // 用于在3D场景中绘制文字或图形
  void ModifyScene(const mjModel* model, const mjData* data,
                   mjvScene* scene) const override;

 protected:
  std::unique_ptr<mjpc::ResidualFn> ResidualLocked() const override {
    return std::make_unique<ResidualFn>(this);
  }
  ResidualFn* InternalResidual() override { return &residual_; }

 private:
  ResidualFn residual_;
};

}  // namespace mjpc

#endif  // MJPC_TASKS_SIMPLE_CAR_SIMPLE_CAR_H_