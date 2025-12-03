/*
  sensors.h - Integração de IMU, PMIC, RTC ao humor do WavePwn
*/

#pragma once

// Wake por movimento (IMU QMI8658) + outros sensores
void init_motion_wakeup(void);