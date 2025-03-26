/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AP_VideoTX.h"

#if AP_VIDEOTX_ENABLED

#include <AP_RCTelemetry/AP_CRSF_Telem.h>
#include <GCS_MAVLink/GCS.h>

#include <AP_HAL/AP_HAL.h>
#include "models/Factory.cpp"

extern const AP_HAL::HAL& hal;

AP_VideoTX *AP_VideoTX::singleton;

const AP_Param::GroupInfo AP_VideoTX::var_info[] = {

    // @Param: ENABLE
    // @DisplayName: Is the Video Transmitter enabled or not
    // @Description: Toggles the Video Transmitter on and off
    // @Values: 0:Disable,1:Enable
    AP_GROUPINFO_FLAGS("ENABLE", 1, AP_VideoTX, _enabled, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: POWER
    // @DisplayName: Video Transmitter Power Level
    // @Description: Video Transmitter Power Level. Different VTXs support different power levels, the power level chosen will be rounded down to the nearest supported power level
    // @Range: 1 1000
    AP_GROUPINFO("POWER",    2, AP_VideoTX, _power_mw, 0),

    // @Param: CHANNEL
    // @DisplayName: Video Transmitter Channel
    // @Description: Video Transmitter Channel
    // @User: Standard
    // @Range: 0 7
    AP_GROUPINFO("CHANNEL",  3, AP_VideoTX, _channel, 0),

    // @Param: BAND
    // @DisplayName: Video Transmitter Band
    // @Description: Video Transmitter Band
    // @User: Standard
    // @Values: 0:Band A,1:Band B,2:Band E,3:Airwave,4:RaceBand,5:Low RaceBand,6:1G3 Band A,7:1G3 Band B,8:Band X
    AP_GROUPINFO("BAND",  4, AP_VideoTX, _band, 0),

    // @Param: FREQ
    // @DisplayName: Video Transmitter Frequency
    // @Description: Video Transmitter Frequency. The frequency is derived from the setting of BAND and CHANNEL
    // @User: Standard
    // @ReadOnly: True
    // @Range: 1000 6000
    AP_GROUPINFO("FREQ",  5, AP_VideoTX, _frequency_mhz, 0),

    // @Param: OPTIONS
    // @DisplayName: Video Transmitter Options
    // @Description: Video Transmitter Options. Pitmode puts the VTX in a low power state. Unlocked enables certain restricted frequencies and power levels. Do not enable the Unlocked option unless you have appropriate permissions in your jurisdiction to transmit at high power levels. One stop-bit may be required for VTXs that erroneously mimic iNav behaviour.
    // @User: Advanced
    // @Bitmask: 0:Pitmode,1:Pitmode until armed,2:Pitmode when disarmed,3:Unlocked,4:Add leading zero byte to requests,5:Use 1 stop-bit in SmartAudio,6:Ignore CRC in SmartAudio,7:Ignore status updates in CRSF and blindly set VTX options
    AP_GROUPINFO("OPTIONS",  6, AP_VideoTX, _options, 0),

    // @Param: MAX_POWER
    // @DisplayName: Video Transmitter Max Power Level
    // @Description: Video Transmitter Maximum Power Level. Different VTXs support different power levels, this prevents the power aux switch from requesting too high a power level. The switch supports 6 power levels and the selected power will be a subdivision between 0 and this setting.
    // @Range: 25 5000
    AP_GROUPINFO("MAX_POWER", 7, AP_VideoTX, _max_power_mw, 5000),

    AP_GROUPINFO("BUTTON_1", 8, AP_VideoTX, _frequency_buttons[0], 5865),
    AP_GROUPINFO("BUTTON_2", 9, AP_VideoTX, _frequency_buttons[1], 5733),
    AP_GROUPINFO("BUTTON_3", 10, AP_VideoTX, _frequency_buttons[2], 5705),
    AP_GROUPINFO("BUTTON_4", 11, AP_VideoTX, _frequency_buttons[3], 5740),
    AP_GROUPINFO("BUTTON_5", 12, AP_VideoTX, _frequency_buttons[4], 5845),
    AP_GROUPINFO("BUTTON_6", 13, AP_VideoTX, _frequency_buttons[5], 5800),

    AP_GROUPINFO("MODEL_ID", 14, AP_VideoTX, _model_id, 3),

    AP_GROUPEND
};

//#define VTX_DEBUG
#ifdef VTX_DEBUG
# define debug(fmt, args...)	GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX: " fmt "\n", ##args)
#else
# define debug(fmt, args...)	do {} while(0)
#endif

extern const AP_HAL::HAL& hal;

AP_VideoTX::AP_VideoTX()
{
    if (singleton) {
        AP_HAL::panic("Too many VTXs");
        return;
    }
    singleton = this;

    AP_Param::setup_object_defaults(this, var_info);

    _model = Factory::default_model();
}

AP_VideoTX::~AP_VideoTX(void)
{
    singleton = nullptr;
}

bool AP_VideoTX::init(void)
{
    if (_initialized) {
        return false;
    }

    // PARAMETER_CONVERSION - Added: Sept-2022
    _options.convert_parameter_width(AP_PARAM_INT16);

    // always the first level of power
    _current_power = 0;
    _power_mw.set_and_save(get_power_mw());
    _current_band = _band;
    _current_channel = _channel;
    _current_frequency = _frequency_mhz;
    _current_options = _options;
    _current_enabled = _enabled;
    _initialized = true;

    return true;
}

uint16_t AP_VideoTX::get_table_frequency_mhz(uint8_t band, uint8_t channel) {
    return _model->getVideoChannels()[band][channel];
}

bool AP_VideoTX::get_band_and_channel(uint16_t freq, VideoBand& band, uint8_t& channel)
{
    for (uint8_t i = 0; i < VTX_MODEL_BANDS; i++) {
        for (uint8_t j = 0; j < VTX_MODEL_CHANNELS; j++) {
            if (_model->getVideoChannels()[i][j] == freq) {
                band = VideoBand(i);
                channel = j;
                return true;
            }
        }
    }
    return false;
}

// set the current power
void AP_VideoTX::set_configured_power_mw(uint16_t power)
{
    _power_mw.set_and_save_ifchanged(power);
}

uint8_t AP_VideoTX::find_current_power() const
{
    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (_power_mw == _model->getPowerLevels()[i].mw) {
            return i;
        }
    }
    return 0;
}

// set the power in dbm, rounding appropriately
void AP_VideoTX::set_power_dbm(uint8_t power, PowerActive active)
{
    if (power == _model->getPowerLevels()[_current_power].dbm
        && _model->getPowerLevels()[_current_power].active == active) {
        return;
    }

    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (power == _model->getPowerLevels()[i].dbm) {
            _current_power = i;
            _model->getPowerLevels()[i].active = active;
            debug("learned power %ddbm", power);
            // now unlearn the "other" power level since we have no other way of guessing
            // the supported levels
            if ((_model->getPowerLevels()[i].level & 0xF0) == 0x10) {
                _model->getPowerLevels()[i].level = _model->getPowerLevels()[i].level & 0xF;
            }
            if (i > 0 && _model->getPowerLevels()[i-1].level == _model->getPowerLevels()[i].level) {
                debug("invalidated power %dwm, level %d is now %dmw", _model->getPowerLevels()[i-1].mw, _model->getPowerLevels()[i].level, _model->getPowerLevels()[i].mw);
                _model->getPowerLevels()[i-1].level = 0;
                _model->getPowerLevels()[i-1].active = PowerActive::Inactive;
            } else if (i < VTX_MAX_POWER_LEVELS-1 && _model->getPowerLevels()[i+1].level == _model->getPowerLevels()[i].level) {
                debug("invalidated power %dwm, level %d is now %dmw", _model->getPowerLevels()[i+1].mw, _model->getPowerLevels()[i].level, _model->getPowerLevels()[i].mw);
                _model->getPowerLevels()[i+1].level = 0;
                _model->getPowerLevels()[i+1].active = PowerActive::Inactive;
            }
            return;
        }
    }
    // learn the non-standard power
    _current_power = update_power_dbm(power, active);
}

// add an active power setting in dbm
uint8_t AP_VideoTX::update_power_dbm(uint8_t power, PowerActive active)
{
    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (power == _model->getPowerLevels()[i].dbm) {
            if (_model->getPowerLevels()[i].active != active) {
                _model->getPowerLevels()[i].active = active;
                debug("%s power %ddbm", active == PowerActive::Active ? "learned" : "invalidated", power);
            }
            return i;
        }
    }
    // handed a non-standard value, use the last slot
    _model->getPowerLevels()[VTX_MAX_POWER_LEVELS-1].dbm = power;
    _model->getPowerLevels()[VTX_MAX_POWER_LEVELS-1].level = 255;
    _model->getPowerLevels()[VTX_MAX_POWER_LEVELS-1].dac = 255;
    _model->getPowerLevels()[VTX_MAX_POWER_LEVELS-1].mw = uint16_t(roundf(powf(10, power / 10.0f)));
    _model->getPowerLevels()[VTX_MAX_POWER_LEVELS-1].active = active;
    debug("non-standard power %ddbm -> %dmw", power, _model->getPowerLevels()[VTX_MAX_POWER_LEVELS-1].mw);
    return VTX_MAX_POWER_LEVELS-1;
}

// add all active power setting in dbm
void AP_VideoTX::update_all_power_dbm(uint8_t nlevels, const uint8_t power[])
{
    for (uint8_t i = 0; i < nlevels; i++) {
        update_power_dbm(power[i], PowerActive::Active);
    }
    // invalidate the remaining ones
    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (_model->getPowerLevels()[i].active == PowerActive::Unknown) {
            _model->getPowerLevels()[i].active = PowerActive::Inactive;
        }
    }
}

// set the power in mw
void AP_VideoTX::set_power_mw(uint16_t power)
{
    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (power == _model->getPowerLevels()[i].mw) {
            _current_power = i;
            break;
        }
    }
}

// set the power "level"
void AP_VideoTX::set_power_level(uint8_t level, PowerActive active)
{
    if (level == _model->getPowerLevels()[_current_power].level
        && _model->getPowerLevels()[_current_power].active == active) {
        return;
    }

    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (level == _model->getPowerLevels()[i].level) {
            _current_power = i;
            _model->getPowerLevels()[i].active = active;
            debug("learned power level %d: %dmw", level, get_power_mw());
            break;
        }
    }
}

// set the power dac
void AP_VideoTX::set_power_dac(uint16_t power, PowerActive active)
{
    if (power == _model->getPowerLevels()[_current_power].dac
        && _model->getPowerLevels()[_current_power].active == active) {
        return;
    }

    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (power == _model->getPowerLevels()[i].dac) {
            _current_power = i;
            _model->getPowerLevels()[i].active = active;
            debug("learned power %dmw", get_power_mw());
        }
    }
}

// set the current channel
void AP_VideoTX::set_enabled(bool enabled)
{
    _current_enabled = enabled;
    if (!_enabled.configured()) {
        _enabled.set_and_save(enabled);
    }
}

void AP_VideoTX::set_power_is_current()
{
    set_power_dbm(get_configured_power_dbm());
}

void AP_VideoTX::set_freq_is_current()
{
    _current_frequency = _frequency_mhz;
    _current_band = _band;
    _current_channel = _channel;
}

// periodic update
void AP_VideoTX::update(void)
{
    if (!_enabled) {
        return;
    }

    // manipulate pitmode if pitmode-on-disarm or power-on-arm is set
    if (has_option(VideoOptions::VTX_PITMODE_ON_DISARM) || has_option(VideoOptions::VTX_PITMODE_UNTIL_ARM)) {
        if (hal.util->get_soft_armed() && has_option(VideoOptions::VTX_PITMODE)) {
            _options.set(_options & ~uint8_t(VideoOptions::VTX_PITMODE));
        } else if (!hal.util->get_soft_armed() && !has_option(VideoOptions::VTX_PITMODE)
            && has_option(VideoOptions::VTX_PITMODE_ON_DISARM)) {
            _options.set(_options | uint8_t(VideoOptions::VTX_PITMODE));
        }
    }
    // check that the requested power is actually allowed
    // reset if not
    if (_power_mw != get_power_mw()) {
        if (_model->getPowerLevels()[find_current_power()].active == PowerActive::Inactive) {
            // reset to something we know works
            debug("power reset to %dmw from %dmw", get_power_mw(), _power_mw.get());
            _power_mw.set_and_save(get_power_mw());
        }
    }
}

bool AP_VideoTX::update_options() const
{
    if (!_defaults_set) {
        return false;
    }
    // check pitmode
    if ((_options & uint8_t(VideoOptions::VTX_PITMODE))
        != (_current_options & uint8_t(VideoOptions::VTX_PITMODE))) {
        return true;
    }

#if HAL_CRSF_TELEM_ENABLED
    // using CRSF so unlock is not an option
    if (AP::crsf_telem() != nullptr) {
        return false;
    }
#endif
    // check unlock only
    if ((_options & uint8_t(VideoOptions::VTX_UNLOCKED)) != 0
        && (_current_options & uint8_t(VideoOptions::VTX_UNLOCKED)) == 0) {
        return true;
    }

    // ignore everything else
    return false;
}

bool AP_VideoTX::update_power() const {
    if (!_defaults_set || _power_mw == get_power_mw() || get_pitmode()) {
        return false;
    }
    // check that the requested power is actually allowed
    for (uint8_t i = 0; i < VTX_MAX_POWER_LEVELS; i++) {
        if (_power_mw == _model->getPowerLevels()[i].mw
            && _model->getPowerLevels()[i].active != PowerActive::Inactive) {
            return true;
        }
    }
    // asked for something unsupported - only SA2.1 allows this and will have already provided a list
    return false;
}

bool AP_VideoTX::have_params_changed() const
{
    return _enabled
        && (update_power()
        || update_band()
        || update_channel()
        || update_frequency()
        || update_options());
}

// update the configured frequency to match the channel and band
void AP_VideoTX::update_configured_frequency()
{
    _frequency_mhz.set_and_save(get_table_frequency_mhz(_band, _channel));
}

// update the configured channel and band to match the frequency
void AP_VideoTX::update_configured_channel_and_band()
{
    VideoBand band;
    uint8_t channel;
    if (get_band_and_channel(_frequency_mhz, band, channel)) {
        _band.set_and_save(band);
        _channel.set_and_save(channel);
    } else {
        update_configured_frequency();
    }
}

// set the current configured values if not currently set in storage
// this is necessary so that the current settings can be seen
bool AP_VideoTX::set_defaults()
{
    if (_defaults_set) {
        return false;
    }

    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX: Model ID %d", _model_id.get());
    _model = Factory::by_model_id_param(_model_id);

    // check that our current view of frequency matches band/channel
    // if not then force one to be correct
    uint16_t calced_freq = get_table_frequency_mhz(_current_band, _current_channel);
    if (_current_frequency != calced_freq) {
        if (_current_frequency > 0) {
            VideoBand band;
            uint8_t channel;
            if (get_band_and_channel(_current_frequency, band, channel)) {
                _current_band = band;
                _current_channel = channel;
            } else {
                _current_frequency = calced_freq;
            }
        } else {
            _current_frequency = calced_freq;
        }
    }

    if (!_options.configured()) {
        _options.set_and_save(_current_options);
    }
    if (!_channel.configured()) {
        _channel.set_and_save(_current_channel);
    }
    if (!_band.configured()) {
        _band.set_and_save(_current_band);
    }
    if (!_power_mw.configured()) {
        _power_mw.set_and_save(get_power_mw());
    }
    if (!_frequency_mhz.configured()) {
        _frequency_mhz.set_and_save(_current_frequency);
    }

    // Now check that the user didn't screw up by selecting incompatible options
    if (_frequency_mhz != get_table_frequency_mhz(_band, _channel)) {
        if (_frequency_mhz > 0) {
            update_configured_channel_and_band();
        } else {
            update_configured_frequency();
        }
    }

    _defaults_set = true;

    announce_vtx_settings();

    return true;
}

void AP_VideoTX::announce_vtx_settings() const
{
    // Output a friendly message so the user knows the VTX has been detected
    GCS_SEND_TEXT(
        MAV_SEVERITY_INFO,
        "%s: %s%d %dMHz, PWR: %dmW",
        _model->getName(),
        _model->getBandNames()[_band.get()],
        _channel.get() + 1,
        _frequency_mhz.get(),
        has_option(VideoOptions::VTX_PITMODE) ? 0 : _power_mw.get()
    );
}

// change the video power based on switch input
// 6-pos range is in the middle of the available range
void AP_VideoTX::change_power(uint8_t position)
{
    if (!_enabled || position > VTX_MAX_POWER_LEVELS) {
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX: Invalid power position: %d", position);
        return;
    }

    uint16_t requested_power = _model->getPowerLevels()[position].mw;
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX: Setting power to: %dmW at position: %d", requested_power, position);

    if (requested_power == 0) {
        if (!hal.util->get_soft_armed()) {    // don't allow pitmode to be entered if already armed
            set_configured_options(get_configured_options() | uint8_t(VideoOptions::VTX_PITMODE));
            GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX: Entering Pitmode");
        }
    } else {
        if (has_option(VideoOptions::VTX_PITMODE)) {
            set_configured_options(get_configured_options() & ~uint8_t(VideoOptions::VTX_PITMODE));
            GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX: Exiting Pitmode");
        }
        set_configured_power_mw(requested_power);
    }
}

namespace AP {
    AP_VideoTX& vtx() {
        return *AP_VideoTX::get_singleton();
    }
};

#endif
