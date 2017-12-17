/*
 * Author: Stephen Leitnick
 * Date: November 2017
 *
 * AlarmSoundFragment simply stores a tone
 * value and a tone duration. It essentially
 * says "play this tone value for this long"
 */

#include "AlarmSoundFragment.h"

AlarmSoundFragment::AlarmSoundFragment(int toneValue, int toneDuration) : value(toneValue), duration(toneDuration) {
	
}

