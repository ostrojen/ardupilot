#include "mode.h"
#include "Plane.h"

bool ModeAttack::_enter()
{
    target_pitch_angle = 0;
    locked_heading = false;  // Initialize heading lock status
    lock_timer_ms = 0;       // Initialize lock timer

    return true;
}

void ModeAttack::update()
{
    // Check for stick inputs on roll or rudder to unlock heading
    if (plane.channel_roll->get_control_in() != 0 || plane.channel_rudder->get_control_in() != 0) {
        locked_heading = false;
        lock_timer_ms = 0;  // Reset lock timer when stick input is detected
    }

    // Apply roll control: if heading is locked, use calculated nav_roll, else use stick input
    if (!locked_heading) {
        plane.nav_roll_cd = plane.channel_roll->norm_input() * plane.roll_limit_cd;
        plane.update_load_factor();
    } else {
        plane.calc_nav_roll();
    }

    // Pitch control logic remains the same
    float pitch_input = plane.channel_pitch->norm_input();
    int32_t current_pitch_angle = plane.ahrs.pitch_sensor;

    if (fabsf(pitch_input) > 0.05) {
        if (pitch_input > 0) {
            target_pitch_angle = current_pitch_angle + pitch_input * (plane.aparm.pitch_limit_max / 4);
        } else {
            target_pitch_angle = current_pitch_angle + pitch_input * (plane.aparm.pitch_limit_min / 4);
        }
        plane.nav_pitch_cd = target_pitch_angle;
    } else {
        plane.nav_pitch_cd = target_pitch_angle;
    }

    plane.adjust_nav_pitch_throttle();
    plane.nav_pitch_cd = constrain_int32(plane.nav_pitch_cd, plane.pitch_limit_min, plane.aparm.pitch_limit_max.get());

    if (plane.fly_inverted()) {
        plane.nav_pitch_cd = -plane.nav_pitch_cd;
    }

    // Throttle control remains the same
    float throttle_input = plane.channel_throttle->norm_input();
    float throttle_output = constrain_float(throttle_input, 0.0f, 1.0f);
    SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, throttle_output * 100);

    // Arming force in air remains the same
    const RC_Channel *arm_channel = rc().find_channel_for_option(RC_Channel::AUX_FUNC::ARMDISARM);
    if (!plane.arming.is_armed() && throttle_input < 0.0f && arm_channel != nullptr && arm_channel->get_aux_switch_pos() == RC_Channel::AuxSwitchPos::HIGH && plane.g.arming_air_enable.get()) {
        plane.arming.arm(AP_Arming::Method::AUXSWITCH, false);
    }
    // Ensure navigate is called each update to manage heading locking
    navigate();
}

void ModeAttack::navigate()
{
    if (!locked_heading &&
        plane.channel_roll->get_control_in() == 0 &&
        plane.channel_rudder->get_control_in() == 0 &&
        lock_timer_ms == 0) {
        // Start the lock timer if sticks are neutral
        lock_timer_ms = AP_HAL::millis();
    }
    
    if (lock_timer_ms != 0 &&
        (AP_HAL::millis() - lock_timer_ms) > 500) {
        // Lock the heading after 0.5 seconds of neutral input
        locked_heading = true;
        lock_timer_ms = 0;
        locked_heading_cd = plane.ahrs.yaw_sensor;  // Set locked heading to current yaw
        plane.prev_WP_loc = plane.current_loc;  // Set waypoint location to current location
    }

    if (locked_heading) {
        plane.next_WP_loc = plane.prev_WP_loc;
        plane.nav_controller->update_heading_hold(locked_heading_cd);  // Hold the locked heading
    }
}