#pragma once
#ifndef _ADMITTANCE_CONTROLLER_H_
#define _ADMITTANCE_CONTROLLER_H_

#include <RobotUtilities/TimerLinux.h>
#include <RobotUtilities/utilities.h>

#include <Eigen/Geometry>
#include <chrono>
#include <deque>
#include <fstream>

class AdmittanceController {
 public:
  struct AdmittanceControllerConfig {
    double dt{0.002};  // used for integration/differentiation
    bool log_to_file{false};
    std::string log_file_path{""};

    struct ComplianceParameters6d {
      // Admittance parameters
      RUT::Matrix6d stiffness{};
      RUT::Matrix6d damping{};
      RUT::Matrix6d inertia{};
    };
    ComplianceParameters6d compliance6d{};

    struct PIDGains {
      double P_trans{0.0};
      double I_trans{0.0};
      double D_trans{0.0};

      double P_rot{0.0};
      double I_rot{0.0};
      double D_rot{0.0};
    };
    PIDGains direct_force_control_gains{};
    RUT::Vector6d direct_force_control_I_limit{};
  };

  AdmittanceController();
  ~AdmittanceController();

  /**
   * @brief      initialize the controller.
   *
   * @param[in]  time0         The time point to start ticking from.
   * @param[in]  config        The struct contains all configs.
   * @param[in]  pose_current  The current robot pose (tool frame)
   *
   * @return     True if successfully initialized.
   */
  bool init(const RUT::TimePoint& time0,
            const AdmittanceControllerConfig& config,
            const RUT::Vector7d& pose_current);

  /**
   * @brief      Sets the robot status.
   *
   * @param[in]  pose_WT    The current tool frame pose in the world frame.
   * @param[in]  wrench_WT  The tool wrench feedback.
   */
  void setRobotStatus(const RUT::Vector7d& pose_WT,
                      const RUT::Vector6d& wrench_T);
  /**
   * @brief      Set the position and force reference (user command).
   *
   * @param[in] pose_WT     tool pose represented in the world frame.
   * @param[in] wrench_WTr  wrench measured in the transformed frame.
   *
   * WARNING remember to call step() after setRobotReference, before
   * setForceControlledAxis. step() will properly update internal states, which
   * is required for setForceControlledAxis to work properly.
   */
  void setRobotReference(const RUT::Vector7d& pose_WT,
                         const RUT::Vector6d& wrench_WTr);
  /**
   * @brief      Sets the force controlled axis.
   *
   * @param[in]  Tr    6x6 orthonormal matrix. Describes the axis direction.
   * @param[in]  n_af  The number of force controlled axes.
   */
  void setForceControlledAxis(const RUT::Matrix6d& Tr, int n_af);
  /**
   * @brief return true if no error.
   */
  int step(RUT::Vector7d& pose);

  /**
   * @brief      Reset all internal states to default. It is recommended to call
   * reset() everytime the robot starts from a complete stop in the air. This
   * includes setting all position offsets/force errors to zero. Call reset()
   * when the next action is computed based on the robot's current pose instead
   *  of being part of a pre-planned trajectory. After a reset(), call
   * setRobotReference() immediately.
   */
  void reset();

 private:
  struct Implementation;
  std::unique_ptr<Implementation> m_impl;
};

#endif  // _ADMITTANCE_CONTROLLER_H_
