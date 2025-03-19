#include "mode.h"
#include "Plane.h"

void ModeFStable::update()
{
    // Stick-based control logic remains the same
    plane.nav_roll_cd  = plane.channel_roll->norm_input() * plane.roll_limit_cd;
    plane.update_load_factor();

    float pitch_input = plane.channel_pitch->norm_input();
    if (pitch_input > 0) {
        plane.nav_pitch_cd = pitch_input * plane.aparm.pitch_limit_max;
    } else {
        plane.nav_pitch_cd = -(pitch_input * plane.pitch_limit_min);
    }

    plane.adjust_nav_pitch_throttle();
    plane.nav_pitch_cd = constrain_int32(plane.nav_pitch_cd, plane.pitch_limit_min, plane.aparm.pitch_limit_max.get());

    if (plane.fly_inverted()) {
        plane.nav_pitch_cd = -plane.nav_pitch_cd;
    }

    // Throttle control logic
    float throttle_input = plane.channel_throttle->norm_input(); // Get throttle input from RC channel
    float throttle_output = constrain_float(throttle_input, 0.0f, 1.0f); // Constrain throttle between 0 and 1
    SRV_Channels::set_output_scaled(SRV_Channel::k_throttle, throttle_output * 100); // Set throttle output (0 to 100%)

    // Check if flight stable failsafe is enabled
    if (plane.g.fstb_failsafe_enable.get() && plane.arming.is_armed() && throttle_input > 0.1f) {
        // Check if time since last valid RC signal exceeds the failsafe timeout
        uint32_t failsafe_ms = static_cast<uint32_t>(plane.g.fstb_failsafe_timeout.get()) * 1000U;
        if ((millis() - plane.failsafe.last_valid_rc_ms) > failsafe_ms) {
            // Assume that we lost the RC signal
            // Activate CRUISE mode upon failsafe
            plane.set_mode(Mode::CRUISE, ModeReason::FAILSAFE);
            return;
        }
    }

    // Arming force in air
    const RC_Channel *arm_channel = rc().find_channel_for_option(RC_Channel::AUX_FUNC::ARMDISARM);
    if (!plane.arming.is_armed() && throttle_input < 0.0f && arm_channel != nullptr && arm_channel->get_aux_switch_pos() == RC_Channel::AuxSwitchPos::HIGH && plane.g.arming_air_enable.get()) {
        plane.arming.arm(AP_Arming::Method::AUXSWITCH, false); // Arming force
    }
}
